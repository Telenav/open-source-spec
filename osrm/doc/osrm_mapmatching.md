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



