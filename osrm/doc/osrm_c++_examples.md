- [Example code from OSRM](#example-code-from-osrm)
  - [std::forward](#stdforward)
  - [Algorithm](#algorithm)
    - [sorting](#sorting)
      - [sort](#sort)
      - [unique](#unique)
      - [nth_element](#nthelement)
    - [Partitioning](#partitioning)
      - [rotate](#rotate)
      - [[shuffle]](#shuffle)
  - [<numeric>](#numeric)
    - [fill](#fill)
    - [accumulate](#accumulate)
    - [adjacent_difference](#adjacentdifference)
    - [partial_sum](#partialsum)
    - [inner_product](#innerproduct)

# Example code from OSRM

## std::forward

Std::forward + [parameter pack] could perfectly forward variable parameter list to target function which could deal with them correctly.

One example is OSRM parameters.    
[osrm::BaseParameters](https://github.com/Telenav/osrm-backend/blob/ce5120f8c1f752c51c931b1ce193809a7aa94749/include/engine/api/base_parameters.hpp#L64) is the one holding all basic information related with GPS trace.  It records coordinate points, bearing, radius, etc.  

In his constructor he assign default value for all its member variables.
```C++
    BaseParameters(const std::vector<util::Coordinate> coordinates_ = {},
                   const std::vector<boost::optional<Hint>> hints_ = {},
                   std::vector<boost::optional<double>> radiuses_ = {},
                   std::vector<boost::optional<Bearing>> bearings_ = {},
                   std::vector<boost::optional<Approach>> approaches_ = {},
                   bool generate_hints_ = true,
                   std::vector<std::string> exclude = {})
        : coordinates(coordinates_), hints(hints_), radiuses(radiuses_), bearings(bearings_),
          approaches(approaches_), exclude(std::move(exclude)), generate_hints(generate_hints_)
    {
    }
```

[RouteParameters](https://github.com/Telenav/osrm-backend/blob/ce5120f8c1f752c51c931b1ce193809a7aa94749/include/engine/api/route_parameters.hpp#L56) inherit from BaseParameters.  He has a default constructor in case you pass nothing
```C++
    RouteParameters() = default;
```
He also have constructor to forward all information passed to him to his parent BaseParameters
```C++
   template <typename... Args>
    RouteParameters(const bool steps_,
                    const bool alternatives_,
                    const bool annotations_,
                    const GeometriesType geometries_,
                    const OverviewType overview_,
                    const boost::optional<bool> continue_straight_,
                    Args... args_)
        : BaseParameters{std::forward<Args>(args_)...}, steps{steps_}, alternatives{alternatives_},
          number_of_alternatives{alternatives_ ? 1u : 0u}, annotations{annotations_},
          annotations_type{annotations_ ? AnnotationsType::All : AnnotationsType::None},
          geometries{geometries_}, overview{overview_}, continue_straight{continue_straight_},
          waypoints()

    {
    }
    //[Perry] Please notice that, C++ only allow last parameter has default value, 
    //        which means everything before args should be properly constructed.
```

[MatchParameters](https://github.com/Telenav/osrm-backend/blob/ce5120f8c1f752c51c931b1ce193809a7aa94749/include/engine/api/match_parameters.hpp#L51) Inherits from RouteParameters, which add additional information related with map matching, such as time stamp.  In its constructor, it will forward everything not related with himself to its parents
```C++
    template <typename... Args>
    MatchParameters(std::vector<unsigned> timestamps_,
                    GapsType gaps_,
                    bool tidy_,
                    std::vector<std::size_t> waypoints_,
                    Args... args_)
        : RouteParameters{std::forward<Args>(args_)..., waypoints_},
          timestamps{std::move(timestamps_)}, gaps(gaps_), tidy(tidy_)
    {
    }

         //[Perry] Please notice that, it also implement default constructor.  
         //        It will call the constructor of RouteParameters with nothing be forwarded to BaseParameters 
         
    MatchParameters()
        : RouteParameters(false,
                          false,
                          false,
                          RouteParameters::GeometriesType::Polyline,
                          RouteParameters::OverviewType::Simplified,
                          {}),
          gaps(GapsType::Split), tidy(false)
    {
    }
```
Here I wrote some simple could to experiment this part: cpp.sh/8kv3r

Here is where [MatchParameters is constructed](https://github.com/Telenav/osrm-backend/blob/ce5120f8c1f752c51c931b1ce193809a7aa94749/include/nodejs/node_osrm_support.hpp#L1418).  OSRM create a MatchParameters without any parameters, and then directly construct its members. 




## Algorithm

### sorting

#### sort

```C++
//RandIt first, RandIt last
std::sort(first, last, [](auto lhs, auto rhs) { return lhs.node < rhs.node; });
```

#### [unique](https://en.cppreference.com/w/cpp/algorithm/unique)

```
// [Perry] unique should pair with sort
std::unique(first, last, [](auto lhs, auto rhs) { return lhs.node == rhs.node; }
```


#### nth_element

Important example in [reorderFirstLast](https://github.com/Telenav/osrm-backend/blob/ce5120f8c1f752c51c931b1ce193809a7aa94749/include/partitioner/reorder_first_last.hpp#L26-L27) demo's the usage of std::nth_element
```C++
// Reorders the first n elements in the range to satisfy the comparator,
// and the last n elements to satisfy the comparator with arguments flipped.
// Note: no guarantees to the element's ordering inside the reordered ranges.
template <typename RandomIt, typename Comparator>
void reorderFirstLast(RandomIt first, RandomIt last, std::size_t n, Comparator comp)
{
    BOOST_ASSERT_MSG(n <= (last - first) / std::size_t{2}, "overlapping subranges not allowed");

    if (n == 0 || (last - first < 2))
        return;

    // Reorder first n: guarantees that the predicate holds for the first elements.
    std::nth_element(first, first + (n - 1), last, comp);

    // Reorder last n: guarantees that the flipped predicate holds for the last k elements.
    // We reorder from the end backwards up to the end of the already reordered range.
    // We can not use std::not2, since then e.g. std::less<> would lose its irreflexive
    // requirements.
    std::reverse_iterator<RandomIt> rfirst{last}, rlast{first + n};

    const auto flipped = [](auto fn) {
        return [fn](auto &&lhs, auto &&rhs) {
            return fn(std::forward<decltype(lhs)>(rhs), std::forward<decltype(rhs)>(lhs));
        };
    };

    std::nth_element(rfirst, rfirst + (n - 1), rlast, flipped(comp));
}
```



### Partitioning

#### [rotate](https://en.cppreference.com/w/cpp/algorithm/rotate)

[code example](https://github.com/Telenav/osrm-backend/blob/ce5120f8c1f752c51c931b1ce193809a7aa94749/src/engine/plugins/trip.cpp#L260)
```C++
auto desired_start_index = std::find(std::begin(duration_trip), std::end(duration_trip), 0);
std::rotate(std::begin(duration_trip), desired_start_index, std::end(duration_trip));
```

#### [shuffle]

[code example](https://github.com/Telenav/osrm-backend/blob/feature/elk-docker-compose/src/benchmarks/packed_vector.cpp#L32-L35)
```C++
    std::vector<std::size_t> indices(num_entries);
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 g(1337);
    std::shuffle(indices.begin(), indices.end(), g);
```

## <numeric>

### fill

```C++
    std::fill(flags.begin() + edges_size - new_edges_size, flags.end(), flag);
```


### accumulate

```C++
        const auto total_step_count =
            std::accumulate(legs.begin(), legs.end(), 0, [](const auto &v, const auto &leg) {
                return v + leg.steps.size();
            });
```

### adjacent_difference

```C++
    std::adjacent_difference(timestamps.begin(), timestamps.end(), sample_times.begin());
```


### partial_sum
```C++
    // inplace prefix sum
    std::partial_sum(turn_lane_offsets.begin(), turn_lane_offsets.end(), turn_lane_offsets.begin());
```

### inner_product

```C++
    double primary_sq_sum = std::inner_product(
        timings_vector.begin(), timings_vector.end(), timings_vector.begin(), 0.0);
```



