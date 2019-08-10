# OSRM Map Matching
Map matching matches/snaps given GPS points to the road network in the most plausible way.


## General description
Latest OSRM map matching API could be found [here](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/http.md#match-service), the official API wiki is [here](http://project-osrm.org/docs/v5.5.1/api/#match-service).  

```
/match/v1/{profile}/{coordinates}?steps={true|false}&geometries={polyline|polyline6|geojson}&overview={simplified|full|false}&annotations={true|false}
```

OSRM expect user input a list of GPS points and GPS precision, returns one or several OSRM route legs to represent matched result.  Consider the problem is finding most probable result based on given observation, and input always contains contains noise and sparseness, matcher problem be transformed to [hidden markov model(HMM)](https://www.microsoft.com/en-us/research/publication/hidden-markov-map-matching-noise-sparseness).  OSRM would weight each candidate/candidate pair base on emission probability and transition probability, then use Viterbi algorithm to accumulate result and calculate the best result.
For back ground related with cloud map matching, please go to [map matching basic description](../../routing_basic/doc/mapmatching_basic.md).

