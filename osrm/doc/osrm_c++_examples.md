- [Example code from OSRM](#example-code-from-osrm)
  - [std::forward](#stdforward)


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



