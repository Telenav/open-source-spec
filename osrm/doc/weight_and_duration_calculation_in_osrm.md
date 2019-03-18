<!-- TOC -->

- [Weight and Duration Calculation in OSRM](#weight-and-duration-calculation-in-osrm)
    - [Basic Concepts](#basic-concepts)
    - [Process Speed/Rate in Lua Profiles](#process-speedrate-in-lua-profiles)
        - [process_way](#process_way)
            - [WayHandlers.ferries](#wayhandlersferries)
            - [WayHandlers.movables](#wayhandlersmovables)
            - [WayHandlers.speed](#wayhandlersspeed)
            - [WayHandlers.surface](#wayhandlerssurface)
            - [WayHandlers.maxspeed](#wayhandlersmaxspeed)
            - [WayHandlers.penalties](#wayhandlerspenalties)
    - [Construct and Store Weight/Duration for NodeBasedEdge](#construct-and-store-weightduration-for-nodebasededge)
    - [Compress NodeBasedEdge](#compress-nodebasededge)

<!-- /TOC -->

# Weight and Duration Calculation in OSRM    
This document will try to explain how does [OSRM](https://github.com/Telenav/osrm-backend) calculate **weight** and **duration** of a route.     
- This document will focus on how to calculate weight and duration for a route for car, since generating route for car is one of the most important and most complex feature of [OSRM](https://github.com/Telenav/osrm-backend). It means:     
    - Only `car.lua` will be focused on, other profiles will be ignored.     
    - Only default behavior of car related codes will be read, ignore other unrelated codes.     


## Basic Concepts    
|Name       |Description                                                                 |
|-----------|----------------------------------------------------------------------------|
|`speed`    |Unit: km/h. This will result in the best estimated travel times.|
|`duration` |Estimated travel times. <br>Unit: seconds. |
|`rate`     |The `rate` is an abstract measure that you can assign to ways as you like to make some ways preferable to others. Routing will prefer ways with high `rate`. <br>It represents weight per meter. The unit will be meters per second if the `weight` is time based. |
|`weight`   |The `weight` of a way is normally computed as `length` / `rate`. The `weight` can be thought of as the resistance or cost when passing the way. Routing will prefer ways with low `weight`. You can also set the `weight` of a way to a fixed value. In this case it's not calculated based on the `length` or `rate`, and the `rate` is ignored.|
|`weight_name`|Name used in output for the routing `weight` property, i.e. indicating which weight was used. <br>- `weight_name=routability`: For routing based on duration, but weighted for preferring certain roads. <br>- `weight_name=duration`: For shortest duration without penalties for accessibility. <br>- `weight_name=distance`: For shortest distance without penalties for accessibility. |

Refer to [OSRM Profiles - Understanding speed, weight and rate](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/profiles.md#understanding-speed-weight-and-rate) for more explaination of these concepts.    

## Process Speed/Rate in Lua Profiles
It's a good idea to read [OSRM Profiles](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/profiles.md) first. It explains why need profiles and how does it work. Meanwhile, it also describes structure and elements details of profiles.    
A profile describes whether or not it's possible to route along a particular type of way, whether we can pass a particular node, and how quickly we'll be traveling when we do.     
A profile provided functions to process for OSM node/way/turn, refer to [Interaction Between C++ and Lua In OSRM](https://github.com/Telenav/open-source-spec/blob/master/osrm/doc/interaction_between_cpp_and_lua_in_osrm.md) for what are these functions, what they actually do and where they're invoked.     


### process_way
Most of handles in the `Lua` function [`process_way()`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L356) are dealing with whether a way routable. Also, some of them are related on `speed`/`rate` processing.      
In below codes, the values of several OSM keys [Key:highway](https://wiki.openstreetmap.org/wiki/Key:highway), [Key:route](https://wiki.openstreetmap.org/wiki/Key:route) and [Key:bridge](https://wiki.openstreetmap.org/wiki/Key:bridge) have been retrieved first.    

[process_way in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L356)
```lua
function process_way(profile, way, result, relations)

    -- [Jay] retrieve values of `highway`,`bridge`,`route` for post way handlers
    -- data table for storing intermediate values during processing
    local data = {
        -- prefetch tags
        highway = way:get_value_by_key('highway'),
        bridge = way:get_value_by_key('bridge'),
        route = way:get_value_by_key('route')
    }

    handlers = Sequence {
        -- [Jay] define way handlers 
        -- ...
    }

    -- [Jay] run way handlers sequentially
    WayHandlers.run(profile, way, result, data, handlers, relations)

    -- [Jay] others

end
```
Let's look inside `speed`/`rate` related sub-functions of `process_way()` one-by-one.    
There're two kinds of values will be got after these sub-functions:    
- `forward_speed/backward_speed/forward_rate/backward_rate` for a normal and routable way;    
- or `weight/duration` for routable ferry/shuttle_train/movable bridge.     

#### WayHandlers.ferries
- Referenced OSM keys/values:    
    - [Key:route](https://wiki.openstreetmap.org/wiki/Key:route): [route=ferry](https://wiki.openstreetmap.org/wiki/Tag:route%3Dferry), [route=shuttle_train](https://wiki.openstreetmap.org/wiki/Proposed_features/shuttle_train)
        - For [route=ferry](https://wiki.openstreetmap.org/wiki/Tag:route%3Dferry) and [route=shuttle_train](https://wiki.openstreetmap.org/wiki/Proposed_features/shuttle_train), there're pre-defined `route_speed` in `car.lua`. All other values of [Key:route](https://wiki.openstreetmap.org/wiki/Key:route) will be ignored.    
    - [Key:duration](https://wiki.openstreetmap.org/wiki/Key:duration)
        - If [route=ferry](https://wiki.openstreetmap.org/wiki/Tag:route%3Dferry) or [route=shuttle_train](https://wiki.openstreetmap.org/wiki/Proposed_features/shuttle_train) exists,    
            - firstly will try to get value of [Key:duration](https://wiki.openstreetmap.org/wiki/Key:duration) as `duration` directly, since it's **highly recommended** for indicating how long the route takes ('00:05' is 5 minutes, '1:15' an hour fifteen, or '48:00' two days).    
            - otherwise use the pre-defined `route_speed` to calculate `weight`/`duration` later, similar with normal ways.     


- Codes     
    - [WayHandlers.ferries in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L120)
    - [route_speeds definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L191)
```lua
-- handling ferries and piers
function WayHandlers.ferries(profile,way,result,data)
    local route = data.route
    if route then
        local route_speed = profile.route_speeds[route]
        if route_speed and route_speed > 0 then
        local duration  = way:get_value_by_key("duration")
        if duration and durationIsValid(duration) then
        result.duration = math.max( parseDuration(duration), 1 )
        end
        result.forward_mode = mode.ferry
        result.backward_mode = mode.ferry
        result.forward_speed = route_speed
        result.backward_speed = route_speed
        end
    end
end
```
```lua
    route_speeds = {
      ferry = 5,
      shuttle_train = 10
    },
```

#### WayHandlers.movables
- Referenced OSM keys/values:    
    - [Key:bridge](https://wiki.openstreetmap.org/wiki/Key:bridge): [bridge:movable](https://wiki.openstreetmap.org/wiki/Key:bridge:movable)
        - For [bridge:movable](https://wiki.openstreetmap.org/wiki/Key:bridge:movable), there're pre-defined `bridge_speeds` in `car.lua`. All other values of [Key:bridge](https://wiki.openstreetmap.org/wiki/Key:bridge) will be ignored.    
    - [capacity:car](https://wiki.openstreetmap.org/wiki/Key:capacity)
    - [Key:duration](https://wiki.openstreetmap.org/wiki/Key:duration)
        - If [bridge:movable](https://wiki.openstreetmap.org/wiki/Key:bridge:movable) exists and can be passed by car, i.e. [capacity:car](https://wiki.openstreetmap.org/wiki/Key:capacity) valid,    
            - firstly will try to get value of [Key:duration](https://wiki.openstreetmap.org/wiki/Key:duration) as `duration` directly, since it's **highly recommended** for indicating how long the route takes ('00:05' is 5 minutes, '1:15' an hour fifteen, or '48:00' two days).    
            - otherwise use the pre-defined `bridge_speeds` to calculate `weight`/`duration` later, similar with normal ways.     

- Codes      
    - [WayHandlers.movables in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L138)
    - [bridge_speeds definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L196)
```lua
-- handling movable bridges
function WayHandlers.movables(profile,way,result,data)
  local bridge = data.bridge
  if bridge then
    local bridge_speed = profile.bridge_speeds[bridge]
    if bridge_speed and bridge_speed > 0 then
      local capacity_car = way:get_value_by_key("capacity:car")
      if capacity_car ~= 0 then
        result.forward_mode = profile.default_mode
        result.backward_mode = profile.default_mode
        local duration  = way:get_value_by_key("duration")
        if duration and durationIsValid(duration) then
          result.duration = math.max( parseDuration(duration), 1 )
        else
          result.forward_speed = bridge_speed
          result.backward_speed = bridge_speed
        end
      end
    end
  end
end
```
```lua
    bridge_speeds = {
      movable = 5
    },
```

#### WayHandlers.speed
- Referenced OSM keys/values:    
    - [Key:highway](https://wiki.openstreetmap.org/wiki/Key:highway)
        - If value of [Key:highway](https://wiki.openstreetmap.org/wiki/Key:highway) exist in pre-defined `speeds`, prefer to use it.    
        - otherwise use `default_speed` instead if routable.    
- Codes    
    - [WayHandlers.speed in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L271)
    - [Tags.get_constant_by_key_value in tags.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/tags.lua#L123)
    - [Tags.get_forward_backward_by_set in tags.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/tags.lua#L49)
    - [speed definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L140)
    - [access_tag_whitelist definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L67)
    - [access_tag_blacklist definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L77)
    - [default_speed definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L33)
    - [access_tags_hierarchy definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L101)
```lua
-- handle speed (excluding maxspeed)
function WayHandlers.speed(profile,way,result,data)
  if result.forward_speed ~= -1 then
    return        -- abort if already set, eg. by a route
  end

  local key,value,speed = Tags.get_constant_by_key_value(way,profile.speeds)

  if speed then
    -- set speed by way type
    result.forward_speed = speed
    result.backward_speed = speed
  else
    -- Set the avg speed on ways that are marked accessible
    if profile.access_tag_whitelist[data.forward_access] then
      result.forward_speed = profile.default_speed
    elseif data.forward_access and not profile.access_tag_blacklist[data.forward_access] then
      result.forward_speed = profile.default_speed -- fallback to the avg speed if access tag is not blacklisted
    elseif not data.forward_access and data.backward_access then
       result.forward_mode = mode.inaccessible
    end

    if profile.access_tag_whitelist[data.backward_access] then
      result.backward_speed = profile.default_speed
    elseif data.backward_access and not profile.access_tag_blacklist[data.backward_access] then
      result.backward_speed = profile.default_speed -- fallback to the avg speed if access tag is not blacklisted
    elseif not data.backward_access and data.forward_access then
       result.backward_mode = mode.inaccessible
    end
  end

  if result.forward_speed == -1 and result.backward_speed == -1 and result.duration <= 0 then
    return false
  end
end
```
```lua
    -- [Jay] i.e. static speeds if no better choice
    speeds = Sequence {
      highway = {
        motorway        = 90,
        motorway_link   = 45,
        trunk           = 85,
        trunk_link      = 40,
        primary         = 65,
        primary_link    = 30,
        secondary       = 55,
        secondary_link  = 25,
        tertiary        = 40,
        tertiary_link   = 20,
        unclassified    = 25,
        residential     = 25,
        living_street   = 10,
        service         = 15
      }
    },
```
```lua
    -- [Jay] default_speed for routable ways which value of Key:highway is not exist in speeds
    default_speed             = 10,
```

#### WayHandlers.surface
- Referenced OSM keys/values:    
  - [Key:surface](https://wiki.openstreetmap.org/wiki/Key:surface), [Key:tracktype](https://wiki.openstreetmap.org/wiki/Key:tracktype), [Key:smoothness](https://wiki.openstreetmap.org/wiki/Key:smoothness)
    - Limit speeds by these values and pre-defined surface_speeds,tracktype_speeds,smoothness_speeds.    
- Codes    
  - [WayHandlers.surface in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L355)
  - [surface_speeds,tracktype_speeds,smoothness_speeds definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L200)
```lua
-- reduce speed on bad surfaces
function WayHandlers.surface(profile,way,result,data)
  local surface = way:get_value_by_key("surface")
  local tracktype = way:get_value_by_key("tracktype")
  local smoothness = way:get_value_by_key("smoothness")

  if surface and profile.surface_speeds[surface] then
    result.forward_speed = math.min(profile.surface_speeds[surface], result.forward_speed)
    result.backward_speed = math.min(profile.surface_speeds[surface], result.backward_speed)
  end
  if tracktype and profile.tracktype_speeds[tracktype] then
    result.forward_speed = math.min(profile.tracktype_speeds[tracktype], result.forward_speed)
    result.backward_speed = math.min(profile.tracktype_speeds[tracktype], result.backward_speed)
  end
  if smoothness and profile.smoothness_speeds[smoothness] then
    result.forward_speed = math.min(profile.smoothness_speeds[smoothness], result.forward_speed)
    result.backward_speed = math.min(profile.smoothness_speeds[smoothness], result.backward_speed)
  end
end
```


#### WayHandlers.maxspeed
- Referenced OSM keys/values:    
  - [Key:maxspeed:advisory](https://wiki.openstreetmap.org/wiki/Key:maxspeed:advisory), [Key:maxspeed](https://wiki.openstreetmap.org/wiki/Key:maxspeed)
    - Prefer to use 80% of max speed.     
- Codes    
  - [WayHandlers.maxspeed in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L434)
  - [Tags.get_forward_backward_by_set in tags.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/tags.lua#L49)
  - [speed_reduction definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L37)
```lua
-- maxspeed and advisory maxspeed
function WayHandlers.maxspeed(profile,way,result,data)
  local keys = Sequence { 'maxspeed:advisory', 'maxspeed' }
  local forward, backward = Tags.get_forward_backward_by_set(way,data,keys)
  forward = WayHandlers.parse_maxspeed(forward,profile)
  backward = WayHandlers.parse_maxspeed(backward,profile)

  if forward and forward > 0 then
    result.forward_speed = forward * profile.speed_reduction
  end

  if backward and backward > 0 then
    result.backward_speed = backward * profile.speed_reduction
  end
end
```
```lua
    -- [Jay] reduce maxspeed factor
    speed_reduction           = 0.8,
```


#### WayHandlers.penalties
- Referenced OSM keys/values:    
  - [Key:service](https://wiki.openstreetmap.org/wiki/Key:service): [Tag:service=alley](https://wiki.openstreetmap.org/wiki/Tag:service%3Dalley), [Tag:service=parking_aisle](https://wiki.openstreetmap.org/wiki/Tag:service%3Dparking_aisle), [Tag:service=driveway](https://wiki.openstreetmap.org/wiki/Tag:service%3Ddriveway), [Tag:service=drive-through](https://wiki.openstreetmap.org/wiki/Tag:service%3Ddrive-through)
  - [Key:width](https://wiki.openstreetmap.org/wiki/Key:width), [Key:lanes](https://wiki.openstreetmap.org/wiki/Key:lanes), [Tag:oneway=alternating](https://wiki.openstreetmap.org/wiki/Tag:oneway=alternating), [Key:side_road](https://wiki.openstreetmap.org/wiki/Key:side_road)     
scale speeds to get better average driving times if these tags exist.    

- Codes    
  - [WayHandlers.penalties in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L375)
  - [service_penalties definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L159)
```lua
-- scale speeds to get better average driving times
function WayHandlers.penalties(profile,way,result,data)
  -- heavily penalize a way tagged with all HOV lanes
  -- in order to only route over them if there is no other option
  local service_penalty = 1.0
  local service = way:get_value_by_key("service")
  if service and profile.service_penalties[service] then
    service_penalty = profile.service_penalties[service]
  end

  local width_penalty = 1.0
  local width = math.huge
  local lanes = math.huge
  local width_string = way:get_value_by_key("width")
  if width_string and tonumber(width_string:match("%d*")) then
    width = tonumber(width_string:match("%d*"))
  end

  local lanes_string = way:get_value_by_key("lanes")
  if lanes_string and tonumber(lanes_string:match("%d*")) then
    lanes = tonumber(lanes_string:match("%d*"))
  end

  local is_bidirectional = result.forward_mode ~= mode.inaccessible and
                           result.backward_mode ~= mode.inaccessible

  if width <= 3 or (lanes <= 1 and is_bidirectional) then
    width_penalty = 0.5
  end

  -- Handle high frequency reversible oneways (think traffic signal controlled, changing direction every 15 minutes).
  -- Scaling speed to take average waiting time into account plus some more for start / stop.
  local alternating_penalty = 1.0
  if data.oneway == "alternating" then
    alternating_penalty = 0.4
  end

  local sideroad_penalty = 1.0
  data.sideroad = way:get_value_by_key("side_road")
  if "yes" == data.sideroad or "rotary" == data.sideroad then
    sideroad_penalty = profile.side_road_multiplier
  end

  local forward_penalty = math.min(service_penalty, width_penalty, alternating_penalty, sideroad_penalty)
  local backward_penalty = math.min(service_penalty, width_penalty, alternating_penalty, sideroad_penalty)

  if profile.properties.weight_name == 'routability' then
    if result.forward_speed > 0 then
      result.forward_rate = (result.forward_speed * forward_penalty) / 3.6
    end
    if result.backward_speed > 0 then
      result.backward_rate = (result.backward_speed * backward_penalty) / 3.6
    end
    if result.duration > 0 then
      result.weight = result.duration / forward_penalty
    end
  end
end
```
```lua
    service_penalties = {
      alley             = 0.5,
      parking           = 0.5,
      parking_aisle     = 0.5,
      driveway          = 0.5,
      ["drive-through"] = 0.5,
      ["drive-thru"] = 0.5
    },
```

## Construct and Store Weight/Duration for NodeBasedEdge
Once `Lua` function [`process_way()`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L356) finished, the `C++` code [`ExtractorCallbacks::ProcessWay()`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/src/extractor/extractor_callbacks.cpp#L92) will be called to handle this way in memory.     
The terminology **NodeBasedEdge** can refer to [Understanding OSRM Graph Representation - Terminology](https://github.com/Telenav/open-source-spec/blob/master/osrm/doc/understanding_osrm_graph_representation.md#terminology).     
For `weight/duration` related handle, there will be 3 steps:    
- [construct `WeightData/DurationData`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/src/extractor/extractor_callbacks.cpp#L126)    
The `WeightData/DurationData` will be represented by [`detail::ByEdgeOrByMeterValue`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/include/extractor/internal_extractor_edge.hpp#L23). It's a tricky way to store a single value but with different labels.      

```c++
void ExtractorCallbacks::ProcessWay(const osmium::Way &input_way, const ExtractionWay &parsed_way)
{

    //[Jay] ignored unrelated codes ... 

    //[Jay] both DurationData and WeightData are alias of detail::ByEdgeOrByMeterValue
    InternalExtractorEdge::DurationData forward_duration_data;
    InternalExtractorEdge::DurationData backward_duration_data;
    InternalExtractorEdge::WeightData forward_weight_data;
    InternalExtractorEdge::WeightData backward_weight_data;

    //[Jay] construct WeightData/DurationData
    //[Jay]  1. the `by_way` means `weight/duration` available for this way(i.e. ferry/shuttle_train/movable bridge), 
    //[Jay]     so store the value for each nodes pair. 
    //[Jay]  2. the `by_meter` means `forward_speed/backward_speed/forward_rate/backward_rate` available for this way, 
    //[Jay]     so store the value with unit meter per second. 
    const auto toValueByEdgeOrByMeter = [&nodes](const double by_way, const double by_meter) {
        using Value = detail::ByEdgeOrByMeterValue;
        // get value by weight per edge
        if (by_way >= 0)
        {
            // FIXME We divide by the number of edges here, but should rather consider
            // the length of each segment. We would either have to compute the length
            // of the whole way here (we can't: no node coordinates) or push that back
            // to the container and keep a reference to the way.
            const std::size_t num_edges = (nodes.size() - 1);
            return Value(Value::by_edge, by_way / num_edges);
        }
        else
        {
            // get value by deriving weight from speed per edge
            return Value(Value::by_meter, by_meter);
        }
    };

    if (parsed_way.forward_travel_mode != extractor::TRAVEL_MODE_INACCESSIBLE)
    {
        //[Jay] ignored unimportant codes here... 

        forward_duration_data = toValueByEdgeOrByMeter(parsed_way.duration, parsed_way.forward_speed / 3.6);
        forward_weight_data = toValueByEdgeOrByMeter(parsed_way.weight, parsed_way.forward_rate);
    }
    if (parsed_way.backward_travel_mode != extractor::TRAVEL_MODE_INACCESSIBLE)
    {
        //[Jay] ignored unimportant codes here... 

        backward_duration_data = toValueByEdgeOrByMeter(parsed_way.duration, parsed_way.backward_speed / 3.6);
        backward_weight_data = toValueByEdgeOrByMeter(parsed_way.weight, parsed_way.backward_rate);
    }

    //[Jay] ignored unrelated codes ... 
```

- [store `WeightData/DurationData` with `NodeBasedEdge`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/src/extractor/extractor_callbacks.cpp#L432)    

```c++
void ExtractorCallbacks::ProcessWay(const osmium::Way &input_way, const ExtractionWay &parsed_way)
{

    //[Jay] ignored unrelated codes ... 

    if (in_forward_direction)   //[Jay] forward edge
    {

        //[Jay] ignored unrelated codes ... 

        util::for_each_pair(
            nodes.cbegin(),
            nodes.cend(),
            [&](const osmium::NodeRef &first_node, const osmium::NodeRef &last_node) {
                NodeBasedEdgeWithOSM edge = {
                    OSMNodeID{static_cast<std::uint64_t>(first_node.ref())},
                    OSMNodeID{static_cast<std::uint64_t>(last_node.ref())},
                    0,  // weight
                    0,  // duration
                    {}, // geometry id
                    static_cast<AnnotationID>(annotation_data_id),
                    {true,
                     in_backward_direction && !split_edge,
                     split_edge,
                     parsed_way.roundabout,
                     parsed_way.circular,
                     parsed_way.is_startpoint,
                     parsed_way.forward_restricted,
                     road_classification,
                     parsed_way.highway_turn_classification,
                     parsed_way.access_turn_classification}};

                //[Jay] store WeightData/DurationData with NodeBasedEdge in memory
                external_memory.all_edges_list.push_back(InternalExtractorEdge(
                    std::move(edge), forward_weight_data, forward_duration_data, {}));
            });
    }

    if (in_backward_direction && (!in_forward_direction || split_edge)) //[Jay] backward edge if necessary
    {
    
        //[Jay] similar as forward, no need to copy code snip again.
    }


    //[Jay] ignored unrelated codes ... 
```

- [Compute Weight/Duration for `NodeBasedEdge`](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/src/extractor/extraction_containers.cpp#L337)     

```c++
    {
        // Compute edge weights

        //[Jay] ignored not unimportant codes ... 

        //[Jay] weight/duration = distance / meter_per_second
        const auto distance =
            util::coordinate_calculation::greatCircleDistance(source_coord, target_coord);
        const auto weight = edge_iterator->weight_data(distance);
        const auto duration = edge_iterator->duration_data(distance);

        //[Jay] `process_segment()` is not currently used in the car profile.
        ExtractionSegment segment(source_coord, target_coord, distance, weight, duration);
        scripting_environment.ProcessSegment(segment);

        //[Jay] HIGHLIGHT: scale the weight/duration value by multiply 10
        auto &edge = edge_iterator->result;
        edge.weight = std::max<EdgeWeight>(1, std::round(segment.weight * weight_multiplier));
        edge.duration = std::max<EdgeWeight>(1, std::round(segment.duration * 10.));

        //[Jay] ignored not unimportant codes ... 
    }
```

## Compress NodeBasedEdge
There're two actions for handle weight/duration applied during Compress NodeBasedGraph.     
For what compress NodeBasedGraph do, please refer to [Understanding OSRM Graph Representation - Basic Changes of Convert OSM to OSRM Edge-expanded Graph](https://github.com/Telenav/open-source-spec/blob/master/osrm/doc/understanding_osrm_graph_representation.md#basic-changes-of-convert-osm-to-osrm-edge-expanded-graph).     

- [Add Penalty for `traffic_signals`](https://github.com/Project-OSRM/osrm-backend/blob/e86d93760f51304940d55d62c0d47f15094d6712/src/extractor/graph_compressor.cpp#L218)    
Please be aware that comments in [graph_compressor.cpp#L209](https://github.com/Project-OSRM/osrm-backend/blob/e86d93760f51304940d55d62c0d47f15094d6712/src/extractor/graph_compressor.cpp#L209) is not correct. it will be fixed by [PR-5384](https://github.com/Project-OSRM/osrm-backend/pull/5384).    
By current implementation, there'll be `20` penalty added to both weight and duration on each compressible `traffic_signals`.     

```c++
    
    //[Jay] ignored not unimportant codes ... 

    const bool has_node_penalty = traffic_signals.find(node_v) != traffic_signals.end();
    EdgeDuration node_duration_penalty = MAXIMAL_EDGE_DURATION;
    EdgeWeight node_weight_penalty = INVALID_EDGE_WEIGHT;
    if (has_node_penalty) //[Jay] if traffic_signals appeared, add some penalty for it
    {
        // we cannot handle this as node penalty, if it depends on turn direction
        if (fwd_edge_data1.flags.restricted != fwd_edge_data2.flags.restricted)
            continue;

        // generate an artifical turn for the turn penalty generation
        std::vector<ExtractionTurnLeg> roads_on_the_right;
        std::vector<ExtractionTurnLeg> roads_on_the_left;
        ExtractionTurn extraction_turn(0,
                                        2,
                                        false,
                                        true,
                                        false,
                                        false,
                                        TRAVEL_MODE_DRIVING,
                                        false,
                                        false,
                                        1,
                                        0,
                                        0,
                                        0,
                                        0,
                                        false,
                                        TRAVEL_MODE_DRIVING,
                                        false,
                                        false,
                                        1,
                                        0,
                                        0,
                                        0,
                                        0,
                                        roads_on_the_right,
                                        roads_on_the_left);

        //[Jay] call Lua function `process_turn()` to add penalty. 
        //[Jay] But actually only the `traffic_signals` related codes in `process_turn()` will be touched, 
        //[Jay]  since no other parameters pass in. 
        scripting_environment.ProcessTurn(extraction_turn);

        //[Jay] HIGHLIGHT: same as before, scale the weight/duration value by multiply 10
        node_duration_penalty = extraction_turn.duration * 10;
        node_weight_penalty = extraction_turn.weight * weight_multiplier;
    }

    //[Jay] ignored not unimportant codes ... 

```

```lua
function process_turn(profile, turn)

  -- [Jay] ignored not unimportant codes ... 

  if turn.has_traffic_light then
      turn.duration = profile.properties.traffic_light_penalty
  end

  -- [Jay] ignored not unimportant codes ... 

  -- for distance based routing we don't want to have penalties based on turn angle
  if profile.properties.weight_name == 'distance' then
     turn.weight = 0
  else
     turn.weight = turn.duration
  end

  -- [Jay] ignored not unimportant codes ... 

end
```

```lua
  traffic_light_penalty          = 2,
```

- [Sum Weight/Duration of Compressed `NodeBasedEdge`s](https://github.com/Project-OSRM/osrm-backend/blob/e86d93760f51304940d55d62c0d47f15094d6712/src/extractor/graph_compressor.cpp#L286)    

```c++
    //[Jay] ignored not unimportant codes ... 


    // add weight of e2's to e1
    graph.GetEdgeData(forward_e1).weight += forward_weight2;
    graph.GetEdgeData(reverse_e1).weight += reverse_weight2;

    // add duration of e2's to e1
    graph.GetEdgeData(forward_e1).duration += forward_duration2;
    graph.GetEdgeData(reverse_e1).duration += reverse_duration2;

    if (node_weight_penalty != INVALID_EDGE_WEIGHT &&
        node_duration_penalty != MAXIMAL_EDGE_DURATION)
    {
        //[Jay] add weight/duration penalty caused by traffic_signals
        graph.GetEdgeData(forward_e1).weight += node_weight_penalty;
        graph.GetEdgeData(reverse_e1).weight += node_weight_penalty;
        graph.GetEdgeData(forward_e1).duration += node_duration_penalty;
        graph.GetEdgeData(reverse_e1).duration += node_duration_penalty;
    }

    //[Jay] ignored not unimportant codes ... 
```