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

Refer to [OSRM Profiles - Understanding speed, weight and rate](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/profiles.md#understanding-speed-weight-and-rate) for more explaination of these concepts.    

## Process Speed/Rate In Lua Profiles
It's a good idea to read [OSRM Profiles](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/profiles.md) first. It explains why need profiles and how does it work. Meanwhile, it also describes structure and elements details of profiles.    
A profile describes whether or not it's possible to route along a particular type of way, whether we can pass a particular node, and how quickly we'll be traveling when we do.     
A profile provided functions to process for OSM node/way/turn, refer to [Interaction Between C++ and Lua In OSRM - Lua Functions Brief](https://github.com/Telenav/open-source-spec/blob/master/osrm/doc/interaction_between_cpp_and_lua_in_osrm.md#lua-functions-brief) for what are the functions and what they actually do.     


### process_way
Most of handles in the `Lua` function `process_way()` are dealing with whether a way routable. Also, some of them are related on `speed`/`rate` processing.      
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

#### WayHandlers.ferries
- Referenced OSM keys/values:    
    - [Key:route](https://wiki.openstreetmap.org/wiki/Key:route): [route=ferry](https://wiki.openstreetmap.org/wiki/Tag:route%3Dferry), [route=shuttle_train](https://wiki.openstreetmap.org/wiki/Proposed_features/shuttle_train)
        - For [route=ferry](https://wiki.openstreetmap.org/wiki/Tag:route%3Dferry) and [route=shuttle_train](https://wiki.openstreetmap.org/wiki/Proposed_features/shuttle_train), there're pre-defined `route_speed` in `car.lua`. All other values of [Key:route](https://wiki.openstreetmap.org/wiki/Key:route) will be ignored.    
    - [Key:duration](https://wiki.openstreetmap.org/wiki/Key:duration)
        - If [route=ferry](https://wiki.openstreetmap.org/wiki/Tag:route%3Dferry) or [route=shuttle_train](https://wiki.openstreetmap.org/wiki/Proposed_features/shuttle_train) exists,    
            - firstly will try to get value of [Key:duration](https://wiki.openstreetmap.org/wiki/Key:duration) as `duration` directly, since it's **highly recommended** for indicating how long the route takes ('00:05' is 5 minutes, '1:15' an hour fifteen, or '48:00' two days).    
            - otherwise use the pre-defined `route_speed` to calculate `weight`/`duration` later, similar with normal ways.     



[WayHandlers.ferries in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L120)
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
[route_speeds definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L191)
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

[WayHandlers.movables in way_handlers.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/lib/way_handlers.lua#L138)
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
[bridge_speeds definition in car.lua](https://github.com/Project-OSRM/osrm-backend/blob/a1e5061799f1980c64be5afb8a9071d6c68d7164/profiles/car.lua#L196)
```lua
    bridge_speeds = {
      movable = 5
    },
```