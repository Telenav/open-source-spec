

# .osrm.timestamp 
Contains the timestamps of the OSM extract if the header is set.           

## List 

```bash
tar tvf nevada-latest.osrm.timestamp
-rw-rw-r-- 0/0               8 1970-01-01 08:00 osrm_fingerprint.meta
-rw-rw-r-- 0/0               8 1970-01-01 08:00 /common/timestamp.meta
-rw-rw-r-- 0/0              20 1970-01-01 08:00 /common/timestamp
```

## osrm_fingerprint.meta
- [osrm_fingerprint.meta](./fingerprint.md)

## /common/timestamp, /common/timestamp.meta
Stores `timestamp` or `data_version` depends on `osrm-extract -d xxx`. This information will be returned in [`data_version` of route response](https://github.com/Telenav/osrm-backend/blob/master-telenav/docs/http.md#data-version).          

- No `-d` when run `osrm-extract`: will be empty, nothing will be stored.    
- `osrm-extract -d osmosis`: stores the `timestamp` get from `.osm.pbf` file, e.g. `2019-01-24T21:15:02Z`.     
- `osrm-extract -d "user defined data version"`: stores the user defined data version string.    


### Layout
![](graph/map.osrm.timestamp.common.timestamp.png)


### Implementation

