# [class packed_vector](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp)

## Purpose
Covert given list of values into packed values, by which could use need memory and still achieve array's time complexity.  
For example, OSM ids always have large value like {333222, 33434343, 534343}, using packed_vector could convert them to {0, 1, 2}.  Using packed_vector could achieve array's time complexity and also save lots of space.

## Interface
[push_back](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L420) will put given value into packed_vector and return an index which could used for retrieve.  
[operator \[\] ](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L366)could be used to retrieve original value.

## Notes
- Input value must be positive and in certain range
```C++
template <storage::Ownership Ownership>
using PackedOSMIDs = util::detail::PackedVector<OSMNodeID, 33, Ownership>;
```
Definition for OSMIDs indicate that the value for original OSMNodeID should in the range of {0, pow(2, 33) - 1}

- packed_vector will store input value into blocks and fill which based on calling sequences.  Its better to make sure each input of push_back is unique value like here
```C++
template <typename BarrierOutIter, typename TrafficSignalsOutIter, typename PackedOSMIDsT>
void readRawNBGraph(const boost::filesystem::path &path,
                    BarrierOutIter barriers,
                    TrafficSignalsOutIter traffic_signals,
                    std::vector<util::Coordinate> &coordinates,
                    PackedOSMIDsT &osm_node_ids,
                    std::vector<extractor::NodeBasedEdge> &edge_list,
                    std::vector<extractor::NodeBasedEdgeAnnotation> &annotations)
                    {
                        // ...
                        auto decode = [&](const auto &current_node) {
                        coordinates[index].lon = current_node.lon;
                        coordinates[index].lat = current_node.lat;
                        // [Perry] During the iteration of osm node id, the value of current_node.node_id is positive and unique
                        osm_node_ids.push_back(current_node.node_id);
                        index++;
                        };
                        // ...
                    }
```
If you use array like {1,1,1,1,2,2,2} to call push_back, you still could get correct result just have wasted space for recording all values.

## [Unit Test](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/unit_tests/util/packed_vector.cpp#L22)
 - General API
 - Functionality test on different bit range
 - Iterator test
 - Extreme condition test


