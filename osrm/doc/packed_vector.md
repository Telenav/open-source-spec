# [class packed_vector](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp)

## Purpose
Covert given list of values into packed values, allocate only needed memory and still achieve array's time complexity.  osrm::packed_array provide a way to compact target array value into array[uint_64].
Say that we have an array contains value in the range of 16 bits, we could make full use of 64 bit to record 4 value like {uint16, uint16, uint16, uint16} into single uint64_t value.  It‘s like using binary stream to paternize and record all the value.


## Interface
[push_back](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L420) will put given value into packed_vector and return an index which could used for futher logic and retrieve .  
[operator \[\] ](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L366)could be used to retrieve original value.

## Notes
- Its much more easy to design a packed_array for certain value bits like 8, 16, 32，64.  They could fit in 64 bits entirely and no boundary case need to be handled.  As a utility class for wider usage, osrm::packed_array generalize compact strategy to handle any bit range in [1, 64].<br/>
By profiling, node ids from OSM's data are in the range of [0, pow(2,33)].
<br/>
<img src="../references/pictures/packed_vector_example.png" alt="packed_vector_example" width="500"/>
<br/>

### How to set value

Use 33 bits as an example, let's say input is {1597322404, 1432114613, 1939964443, 2112255763}  
The binary for `1597322404` is `001011111001101010011010010100100` in 33 bits, if we use inner array's 64bit value to record which then result is `0000000000000000000000000000000001011111001101010011010010100100`.  
The binary for `1432114613` is `001010101010111000101010110110101` in 33 bits, for previous 64bit value there are still 31 left, so `1010101010111000101010110110101` will be recorded in the first 64 bit value and `00` will be recorded in the second 64bit value.  The final result for inner array's first 64bit value is `1010101010111000101010110110101001011111001101010011010010100100` and second 64bit is still all 0.  
The binary for `1939964443` is `001110011101000011000001000011011` in 33 bits, we could merge the value as a whole part into second 64bit value, so **35 bits**(2 bits from first input, 33 bits from second input) has taken and result is `0000000000000000000000000000000111001110100001100000100001101100`  
The binary for `2112255763` is `001111101111001100111011100010011` in 33 bits, we could merge the lower 19 bits into the free space of second 64bit value and merge remaining part into third 64 bit vlaue.  The result of inner array's second 64bit value is `1110111100110011101110001001100111001110100001100000100001101100` and third 64bit value is `0000000000000000000000000000000000000000000000000000000000000011`.  
By this example you could see that we could use less space to pack more 64 bit value into it.  

### How to retrieve value

Next question is, how to **retrieve** result back?  Our expectation is
```
packed_array[index] == original_input_array[index]
```
packed_array retrieve is the reverse way compare with set.  
For index = 1, we know that it tooks 33 bits from inner array's first value, just use inner_array[0] & `0000000000000000000000000000000111111111111111111111111111111111` you could get original result
For index = 2, we know that it tooks 31 bits from inner array's first value and 2 bits from second, so you could use (inner_array[1] & `0000000000000000000000000000000000000000000000000000000000000011`)<< 31 for the upper part, plus (inner_array[1] & `1111111111111111111111111111111000000000000000000000000000000000`)>>33 for the lower part.

### How to get random index
osrm::packed_array grouped input into 64 elements(BLOCK_ELEMENTS = 64), when reach 64 then start to use new uint_64 to record new value.  
Then OSRM calculate pattern for all 64 situation, I picked 5 elements from each of the array  
```
lower_mask[0]0000000000000000000000000000000111111111111111111111111111111111
lower_mask[1]1111111111111111111111111111111000000000000000000000000000000000
lower_mask[2]0000000000000000000000000000011111111111111111111111111111111100
lower_mask[3]1111111111111111111111111111100000000000000000000000000000000000
lower_mask[4]0000000000000000000000000001111111111111111111111111111111110000

upper_mask[0]0000000000000000000000000000000000000000000000000000000000000000
upper_mask[1]0000000000000000000000000000000000000000000000000000000000000011
upper_mask[2]0000000000000000000000000000000000000000000000000000000000000000
upper_mask[3]0000000000000000000000000000000000000000000000000000000000001111
upper_mask[4]0000000000000000000000000000000000000000000000000000000000000000

lower_offset[0]0000000000000000000000000000000000000000000000000000000000000000
lower_offset[1]0000000000000000000000000000000000000000000000000000000000100001
lower_offset[2]0000000000000000000000000000000000000000000000000000000000000010
lower_offset[3]0000000000000000000000000000000000000000000000000000000000100011
lower_offset[4]0000000000000000000000000000000000000000000000000000000000000100

upper_offset[0]0000000000000000000000000000000000000000000000000000000000100001
upper_offset[1]0000000000000000000000000000000000000000000000000000000000011111
upper_offset[2]0000000000000000000000000000000000000000000000000000000000100001
upper_offset[3]0000000000000000000000000000000000000000000000000000000000011101
upper_offset[4]0000000000000000000000000000000000000000000000000000000000100001
```









- Input value must be **positive** and in certain **bit-range**  
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

- packed_vector uses CAS spinlocks to prevent **data races** in parallel calls.  You could find more details in the function of [set_value](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/util/packed_vector.hpp#L512)



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
