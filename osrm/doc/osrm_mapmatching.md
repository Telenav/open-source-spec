# OSRM Map Matching
Map matching matches/snaps given GPS points to the road network in the most plausible way.


## General description
Latest OSRM map matching API could be found [here](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/http.md#match-service), the official API wiki is [here](http://project-osrm.org/docs/v5.5.1/api/#match-service).  

```
/match/v1/{profile}/{coordinates}?steps={true|false}&geometries={polyline|polyline6|geojson}&overview={simplified|full|false}&annotations={true|false}
```

OSRM expect user input a list of GPS points and GPS precision, returns one or several OSRM route legs to represent matched result.  Consider the problem is finding most probable result based on given observation, and input always contains contains noise and sparseness, matcher problem be transformed to [hidden markov model(HMM)](https://www.microsoft.com/en-us/research/publication/hidden-markov-map-matching-noise-sparseness).  OSRM would weight each candidate/candidate pair base on emission probability and transition probability, then use Viterbi algorithm to accumulate result and calculate the best result.
For back ground related with cloud map matching, please go to [map matching basic description](../../routing_basic/doc/mapmatching_basic.md).



## Work with matcher service


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

Calculate emission prob for all trace_coordinates, each trace_coordinates could have a list of matched candidates.  The definition of strcut EmissionLogProbability could be found [here](https://github.com/Telenav/osrm-backend/blob/7677b8513bf8cdbadb575c745acf4f9124887764/include/engine/map_matching/hidden_markov_model.hpp#L27).

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
// DP array used to record best weight to each candidate
// the bigger the better
std::vector<std::vector<double>> viterbi;

// Is this candidate reachable from previous candidate
// reachable be assigned value during retrieve
std::vector<std::vector<bool>> viterbi_reachable;

// pair = <parent_timestamp_index, parent_candidate_index>
std::vector<std::vector<std::pair<unsigned, unsigned>>> parents;

// accumulate distance
std::vector<std::vector<float>> path_distances;

// Whether this candidate need to be pruned
// Only if he could provide better opportunity 
std::vector<std::vector<bool>> pruned;

// Whether there is a new sub match result
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

## Case learning

### Simple case

### Case with broken trace

### Case with incorrect map data





