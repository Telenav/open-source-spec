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
It's a good idea to read [OSRM Profiles](https://github.com/Project-OSRM/osrm-backend/blob/master/docs/profiles.md) first. It explains why need profiles and how does it works. Meanwhile, it also describes structure and elements details of profiles.    