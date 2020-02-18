

# .osrm.properties 
Contains Lua profile properties that affect run time queries.           

## List 

```bash
tar -tvf nevada-latest.osrm.properties
-rw-rw-r-- 0/0               8 1970-01-01 00:00 osrm_fingerprint.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/properties.meta
-rw-rw-r-- 0/0            2344 1970-01-01 00:00 /common/properties
```

## osrm_fingerprint.meta
- [osrm_fingerprint.meta](./fingerprint.md)

## /common/properties, /common/properties.meta

### Layout
![](./graph/map.osrm.properties.common.properties.png)

### Implementation
The data stored in `/common/properties` is the [`struct ProfileProperties`](https://github.com/Telenav/osrm-backend/blob/8e094d1b90a31c090cdafa9854985343ea1e15c3/include/extractor/profile_properties.hpp#L23) in `C++` implementation.     
- [`struct ProfileProperties`](https://github.com/Telenav/osrm-backend/blob/8e094d1b90a31c090cdafa9854985343ea1e15c3/include/extractor/profile_properties.hpp#L23)'s data values are come from [Lua properties](https://github.com/Telenav/osrm-backend/blob/8e094d1b90a31c090cdafa9854985343ea1e15c3/profiles/car.lua#L16) or default values if it doesn't present in Lua. But be aware that some details maybe special, e.g. `u_turn_penality = lua.properties.u_turn_penality * 10`, since they have different units(deci-seconds in [`struct ProfileProperties`](https://github.com/Telenav/osrm-backend/blob/8e094d1b90a31c090cdafa9854985343ea1e15c3/include/extractor/profile_properties.hpp#L23) but seconds in Lua).     
- Refer to [Sol2ScriptingEnvironment::InitContext()](https://github.com/Telenav/osrm-backend/blob/ec2dab5400a016f905d86e482931b6526b30759b/src/extractor/scripting_environment_lua.cpp#L104) for the loading process from Lua to [`struct ProfileProperties`](https://github.com/Telenav/osrm-backend/blob/8e094d1b90a31c090cdafa9854985343ea1e15c3/include/extractor/profile_properties.hpp#L23). [Interaction Between C++ and Lua In OSRM](https://github.com/Telenav/open-source-spec/blob/master/osrm/doc/interaction_between_cpp_and_lua_in_osrm.md) has more explaination on this part.       
- [osrm::extractor::files::writeProfileProperties()](https://github.com/Telenav/osrm-backend/blob/ec2dab5400a016f905d86e482931b6526b30759b/include/extractor/files.hpp#L71:13) is designed for write the [`struct ProfileProperties`](https://github.com/Telenav/osrm-backend/blob/8e094d1b90a31c090cdafa9854985343ea1e15c3/include/extractor/profile_properties.hpp#L23) into file. It will call the [osrm::storage::tar::detail::WriteFrom()](https://github.com/Telenav/osrm-backend/blob/ec2dab5400a016f905d86e482931b6526b30759b/include/storage/tar.hpp#L293) at underlying which will write `T` to file by taking its address and `sizeof` directly.      