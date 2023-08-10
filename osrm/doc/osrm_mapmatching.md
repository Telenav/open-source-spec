- [OSRM Map Matching](#osrm-map-matching)
  - [General description](#general-description)
  - [Work with matcher service](#work-with-matcher-service)
  - [Code analysis](#code-analysis)
    - [Emission prob](#emission-prob)
    - [Transition prob](#transition-prob)
    - [HMM](#hmm)
    - [Calculate shortest path](#calculate-shortest-path)
    - [Handling Split](#handling-split)
    - [Construct sub-match result](#construct-sub-match-result)
    - [Evaluate confidence](#evaluate-confidence)
  - [Case learning](#case-learning)
    - [Simple case](#simple-case)
    - [Case with broken trace](#case-with-broken-trace)
    - [Case with incorrect map data](#case-with-incorrect-map-data)
  - [Reference](#reference)

# OSRM Map Matching
Map matching matches/snaps given GPS points to the road network in the most plausible way.

## General description
Latest OSRM map matching API could be found [here](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/http.md#match-service), the official API wiki is [here](http://project-osrm.org/docs/v5.5.1/api/#match-service).  

```
/match/v1/{profile}/{coordinates}?steps={true|false}&geometries={polyline|polyline6|geojson}&overview={simplified|full|false}&annotations={true|false}
```

OSRM expect user input a list of GPS points and GPS precision, returns one or several OSRM route legs to represent matched result.  Consider the problem is finding most probable result based on given observation, and input always contains noise and sparseness, matcher problem be transformed to [hidden markov model(HMM)](https://www.microsoft.com/en-us/research/publication/hidden-markov-map-matching-noise-sparseness).  HMM's define a class of models that utilize a directed graph structure to represent the probability of observing a particular sequence of event - or in our case OSRM would weight each candidate/candidate pair base on emission probability and transition probability, then use Viterbi algorithm to accumulate result and calculate the best result.
For back ground related with cloud map matching, please go to [map matching basic description](../../routing_basic/doc/mapmatching_basic.md).



## Work with matcher service
- Clean data.  
  This pre-processing part could take half of your efforts in map matching.  In our past, we always use [Kalman filter](https://blog.maddevs.io/reduce-gps-data-error-on-android-with-kalman-filter-and-accelerometer-43594faed19c) to remove unwanted points.  
  If your GPS trace contains speed and direction, you could also make such simple rule:for all neighboring points we calculate the speed according to the GPS and compare it with the speed obtained from the vehicle. If the difference is greater than the acceptable level of inaccuracy, we throw out one of the points. 

- The header of GPS trace or the end part always not helpful for map matching algorithm, except that your target is identify parking or map making for first mile or last mile.  For ETA and traffic flow, we focus on inner points more - we could throw the beginning and end of the route then add back by route calculation.  We could use speed parameter to cut them - before the vehicle's speed reached 5kph ~ 10kph.  
- OSRM will broken data based on "timestamp", if there is no such field in request OSRM will always try to connect between two points even this two are pretty far away.

- The parameter of *radius* is important for map matching result.  
  If the parameter value is too low, sections of the route can get lost; if it is too high, you get superfluous loops added to the route.  
  It worth to spend time to get one or several value for this parameter based on your gps trace.  At first, I pick several golden cases(100+), by input different value(3, 6, 9) and compare with sum of gps point's distance.  This strategy is helpful at small amount of case and quick start.  
  A better way is using [dynamic time warping](https://en.wikipedia.org/wiki/Dynamic_time_warping) to deal with such case.  We could process 1000+ routes and comaring the results using the DTW algorithm and then determine the best results.



## Code analysis
For each HTTP request, after handled by boost::asio, it will come to this [function](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/src/engine/routing_algorithms/map_matching.cpp#L71)
```c++
template <typename Algorithm>
SubMatchingList mapMatching(SearchEngineData<Algorithm> &engine_working_data,
    const DataFacade<Algorithm> &facade,
    const CandidateLists &candidates_list,
    const std::vector<util::Coordinate> &trace_coordinates,
    const std::vector<unsigned> &trace_timestamps,
    const std::vector<boost::optional<double>> &trace_gps_precision,
    const bool allow_splitting)
```

### Emission prob

Calculate emission prob for all trace_coordinates, each trace_coordinates could have a list of matched candidates.  

```C++
      for (auto t = 0UL; t < candidates_list.size(); ++t)
      {
          emission_log_probabilities[t].resize(candidates_list[t].size());
          std::transform(candidates_list[t].begin(),
                         candidates_list[t].end(),
                         emission_log_probabilities[t].begin(),
                         [&](const PhantomNodeWithDistance &candidate) {
                             return default_emission_log_probability(candidate.distance);
                         });

      }
```

The definition of strcut EmissionLogProbability could be found [here](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/include/engine/map_matching/hidden_markov_model.hpp#L27).

```c++
double operator()(const double distance) const
{
return -0.5 * (log_2_pi + (distance / sigma_z) * (distance / sigma_z)) - log_sigma_z;
}
```
Depend whether "trace_gps_precision" has been set to will use either default sigma_z or adjusted one.


### Transition prob

Here is the logic in function [mapMatching]() to calculate transition prob: 
```C++
// Calculate transition prob between last step(candidates calculated in previous step) and this step
for (const auto s : util::irange<std::size_t>(0UL, prev_viterbi.size()))
{
	for (const auto s_prime : util::irange<std::size_t>(0UL, current_viterbi.size()))
	{
		//…
		double network_distance = getNetworkDistance(engine_working_data,
		facade,
		forward_heap,
		reverse_heap,
		prev_unbroken_timestamps_list[s].phantom_node,
		current_timestamps_list[s_prime].phantom_node,
		weight_upper_bound);
		
		const auto d_t = std::abs(network_distance - haversine_distance);
		
	}
}

```
Transition probability try to identify candidate pair has similar great circle distance between two points and shortest path distance.  The definition of struct TransitionLogProbability could be found [here](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/include/engine/map_matching/hidden_markov_model.hpp#L42).
```C++
double operator()(const double d_t) const { return -log_beta - d_t / beta; }
```


### HMM

Init HMM with candidates list and related emission prob.
```C++
HMM model(candidates_list, emission_log_probabilities);
```

The definition of class HiddenMarkovModel could be found [here](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/src/engine/routing_algorithms/map_matching.cpp#L224)
```C++
template <class CandidateLists> struct HiddenMarkovModel
{
// [Perry]DP array used to record best weight to each candidate
// the bigger the better
std::vector<std::vector<double>> viterbi;

// [Perry]Is this candidate reachable from previous candidate
// reachable be assigned value during retrieve
std::vector<std::vector<bool>> viterbi_reachable;

// [Perry]pair = <parent_timestamp_index, parent_candidate_index>
std::vector<std::vector<std::pair<unsigned, unsigned>>> parents;

// [Perry]accumulate distance
std::vector<std::vector<float>> path_distances;

// [Perry]Whether this candidate need to be pruned
// Only if he could provide better opportunity 
std::vector<std::vector<bool>> pruned;

// [Perry]Whether there is a new sub match result
std::vector<bool> breakage;

```

Viterbi algorithm act as dynamic programming, it accumulate solution from previous step and could always tell the best matched result.  
You could find related logic [here](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/src/engine/routing_algorithms/map_matching.cpp#L235):
```C++
    // [Perry] For current candidate(s_prime), without consider transition prob from s to s_prime, 
    //         the cost is already smaller than current_viterbi[s_prime] which records best 
    //         result be found so far, then this candidate(s_prime) won't be a better solution compare with current_viterbi
    //         Because transition_pr is a negative value, new_value + transition_pr could only be smaller
    double new_value = prev_viterbi[s] + emission_pr;
    if (current_viterbi[s_prime] > new_value)
    {
        continue;
    }
```
and [here](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/src/engine/routing_algorithms/map_matching.cpp#L261)

```C++
        // [Perry] new_value = prev_viterbi[s] + emission_pr + transition_pr
        //         The bigger new_value means more optimal result
        if (new_value > current_viterbi[s_prime])
        {
            current_viterbi[s_prime] = new_value;
            current_parents[s_prime] = std::make_pair(prev_unbroken_timestamp, s);
            current_lengths[s_prime] = network_distance;
            current_pruned[s_prime] = false;
            model.breakage[t] = false;
        }
```

OSRM try to avoid generating too many candidates for DP by pruning some un-reasonable candidates:
```C++
            // assumes minumum of 4 m/s
            const EdgeWeight weight_upper_bound =
                ((haversine_distance + max_distance_delta) / 4.) * facade.GetWeightMultiplier();

            // ...

                    // get distance diff between loc1/2 and locs/s_prime
                    const auto d_t = std::abs(network_distance - haversine_distance);

                    // very low probability transition -> prune
                    if (d_t >= max_distance_delta)
                    {
                        continue;
                    }

```

### Calculate shortest path
OSRM using BFS to calculate shortest path between each candidate pair
```C++
          // [Perry]Here phantom_node means map matching point, which is a point on navigable segment link
          //        from given gps point
          //        weight_upper_bound used to bound routing search radius
          double network_distance =
              getNetworkDistance(engine_working_data,
                                 facade,
                                 forward_heap,
                                 reverse_heap,
                                 prev_unbroken_timestamps_list[s].phantom_node,
                                 current_timestamps_list[s_prime].phantom_node,
                                 weight_upper_bound);
```
In Telenav's previous experience, we tried using different strategy to optimize this part.  Like additional logic to filter or merge phantom_node candidates, or using DFS to replace BFS logic.  One important reason is that we didn't use HMM model before, and both of such strategy will evolve additional logic while mapping to strategy here.  My opinion is, for traffic pattern matching, if we have strict rules(like follow specific pattern of road type) and we have requirement on single map matching result's performance, we could consider to use such optimization strategy.  For user prob matching which is happening on the cloud side, we could partition requests to speed up and choose simple and clear strategy like OSRM or Valhalla map matching.

### Handling Split
When input gps trace contains a gap and hard to merge together, OSRM will consider to generate multiple sub match result.

```C++
      const bool gap_in_trace = [&]() {
          // use temporal information if available to determine a split
          // but do not determine split by timestamps if wasn't asked about it
          if (use_timestamps && allow_splitting)
          {
              return step_time > max_broken_time;
          }
          else
          {
              return t - prev_unbroken_timestamps.back() > MAX_BROKEN_STATES;
          }
      }();
```
When there is a gap, HMM need to be reset and initialized again to calculate next match.

### Construct sub-match result
These [part of code](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/src/engine/routing_algorithms/map_matching.cpp#L328) used to construct final result for map matching

### Evaluate confidence

```C++
    matching.confidence = confidence(trace_distance, matching_distance);
```
Class MatchingConfidence defines strategy for how to evaluate confidence.
```C++
    double operator()(const float trace_length, const float matched_length) const
    {
        const double distance_feature = -std::log(trace_length) + std::log(matched_length);

        // matched to the same point
        if (!std::isfinite(distance_feature))
        {
            return 0;
        }

        const auto label_with_confidence = classifier.classify(distance_feature);
        if (label_with_confidence.first == ClassifierT::ClassLabel::POSITIVE)
        {
            return label_with_confidence.second;
        }

        BOOST_ASSERT(label_with_confidence.first == ClassifierT::ClassLabel::NEGATIVE);
        return 1 - label_with_confidence.second;
    }
```



## Case learning

### Simple case

This case based on Berlin data, I use which to track generic map matching's logic.
```
http://127.0.0.1:5002/match/v1/driving/13.386399,52.517004;13.385498,52.517611;13.385482,52.517915;13.384667,52.518493?steps=true&overview=full
```
In total, there are 4 gps points to be matched.  

The result of calculating emission cost is:
```bash
For point 0 the emission prob is [-2.5307,]
For point 1 the emission prob is [-4.25262,-4.25262,]
For point 2 the emission prob is [-3.72073,-3.72073,-4.04626,-4.04626,-5.32181,-5.32181,-5.32181,-5.32181,]
For point 3 the emission prob is [-2.60078,-2.60078,]
```
The result means for point 0, there are 1 candidate or 1 segment could be the potential solution.  For point 2 have 2 candidates with and for point 3 have 8 candidates.  We also calculate prob result for all candidates.  

Then come into the loop of 
```C++
for (const auto s : util::irange<std::size_t>(0UL, prev_viterbi.size()))
{
	for (const auto s_prime : util::irange<std::size_t>(0UL, current_viterbi.size()))
```


For first round, it will set point 1 as current and point 0 as previous one.
Logs for important variables are:
```
+++prev_viterbi: -2.5307 
+++prev_pruned: 0 
+++prev_unbroken_timestamps_list's size is 1
+++current_viterbi: -inf -inf 
+++current_pruned: 1 1 
+++current_parents: 0,0 0,0 
+++current_lengths: 0 0 
+++current_timestamps_list's size is 2
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-11.7035,-2.5307,-4.25262,-4.92014
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-41.3886,-2.5307,-4.25262,-34.6053
```

For second round, it will set point 2 as current and point 1 as previous
```
+++prev_viterbi: -11.7035 -41.3886 
+++prev_pruned: 0 0 
+++prev_unbroken_timestamps_list's size is 2
+++current_viterbi: -inf -inf -inf -inf -inf -inf -inf -inf 
+++current_pruned: 1 1 1 1 1 1 1 1 
+++current_parents: 0,0 0,0 0,0 0,0 0,0 0,0 0,0 0,0 
+++current_lengths: 0 0 0 0 0 0 0 0 
+++current_timestamps_list's size is 8
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-17.7283,-11.7035,-3.72073,-2.30414
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-55.9615,-11.7035,-3.72073,-40.5373
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-56.3879,-11.7035,-4.04626,-40.6382
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-18.1516,-11.7035,-4.04626,-2.40192
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-20.2219,-11.7035,-5.32181,-3.19661
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-75.2671,-11.7035,-5.32181,-58.2418
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-58.4582,-11.7035,-5.32181,-41.4329
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-20.2219,-11.7035,-5.32181,-3.19661
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-100.366,-41.3886,-3.72073,-55.2566
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-85.468,-41.3886,-4.04626,-40.0332
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-110.2,-41.3886,-5.32181,-63.4895
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-87.5383,-41.3886,-5.32181,-40.8279
```
Please notice that, ideally there should be 2 * 8 = 16 result for this step, due to the existing of weight_upper_bound OSRM
pruned impossible candidates.

Here is the result for the third round:
```
+++prev_viterbi: -17.7283 -55.9615 -56.3879 -18.1516 -20.2219 -75.2671 -58.4582 -20.2219 
+++prev_pruned: 0 0 0 0 0 0 0 0 
+++prev_unbroken_timestamps_list's size is 8
+++current_viterbi: -inf -inf 
+++current_pruned: 1 1 
+++current_parents: 0,0 0,0 
+++current_lengths: 0 0 
+++current_timestamps_list's size is 2
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-96.898,-17.7283,-2.60078,-76.5689
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-26.4898,-17.7283,-2.60078,-6.16071
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-80.0891,-55.9615,-2.60078,-21.5268
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-100.112,-56.3879,-2.60078,-41.1232
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-40.589,-18.1516,-2.60078,-19.8365
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-78.1178,-18.1516,-2.60078,-57.3654
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-43.4539,-20.2219,-2.60078,-20.6312
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-29.879,-20.2219,-2.60078,-7.05629
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-104.057,-20.2219,-2.60078,-81.2345
+++(new_value = prev_viterbi[s] + emission_pr + transition_pr)-71.3333,-20.2219,-2.60078,-48.5107
```


### Case with broken trace

test case
```
http://127.0.0.1:5002/match/v1/driving/13.386399,52.517004;13.385498,52.517611;13.385482,52.517915;13.384667,52.518493;13.562944,52.443846;13.563824,52.444075;13.565816,52.444363?steps=true&overview=full&timestamps=1414131082;1414131083;1414131084;1414131085;1414132082;1414132083;1414132085
```


### Case with incorrect map data



## Reference
- [OSRM issue 2933](https://github.com/Project-OSRM/osrm-backend/issues/2933#issuecomment-249068441)

