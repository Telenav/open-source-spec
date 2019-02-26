# [class packed_vector](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp)

## Purpose
Covert given list of values into packed values, allocate only needed memory and still achieve array's time complexity.  
For example, OSM ids are incontinuous and have value over wide range, packed_vector coverts them to {0, 1, 2 ...} and build mapping internally.

## Interface
[push_back](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L420) will put given value into packed_vector and return an index which could used for futher logic and retrieve .  
[operator \[\] ](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L366)could be used to retrieve original value.

## Notes
- Input value must be positive and in pre-defined range
```C++
template <storage::Ownership Ownership>
using PackedOSMIDs = util::detail::PackedVector<OSMNodeID, 33, Ownership>;
```
Definition for OSMIDs indicate that the value for original OSMNodeID should in the range of {0, pow(2, 33) - 1}

- packed_vector will store input value into blocks and fill which based on calling sequences.  Its better to make sure each input of push_back is unique value.
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
If you use array like {1,1,1,1,2,2,2} to call push_back, you still could get correct result just need spaces inorder to record all the mapping.   

- Internal implementation
```C++
    void push_back(const T value)
    {
        BOOST_ASSERT_MSG(value <= T{(1ULL << Bits) - 1}, "Value too big for packed storage.");

        auto internal_index = get_internal_index(num_elements);

        while (internal_index.lower_word + 1 >= vec.size())
        {
            allocate_blocks(1);
        }

        set_value(internal_index, value);
        num_elements++;

        BOOST_ASSERT(static_cast<T>(back()) == value);
    }
```

- packed_vector uses CAS spinlocks to prevent data races in parallel calls.  You could find more details in the function of [set_value](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L512)



## [Unit Test](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/unit_tests/util/packed_vector.cpp#L22)
 - General API(set and get)
 - Functionality test on different bit range(33, 10, etc)
 - Iterator test(begin, end, forward iterator, backward iterator,)
 - Extreme condition test

<br/>

## Reference
- [boost::any](http://cpp.sh/5savy)
- [boost::iterator_range](http://cpp.sh/23msm)
- [boost range for humans](https://greek0.net/boost-range/)
