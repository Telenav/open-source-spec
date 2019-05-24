# Table service

## API
Table service also be called [Matrix API](https://docs.mapbox.com/help/how-mapbox-works/directions/#mapbox-matrix-api), which is used for calculated travel times/distance between many locations.  

## Examples
Based on the definition in [OSRM's table-service API Documentation](http://project-osrm.org/docs/v5.5.1/api/#table-service), we construct following examples:

### One to many

```
# Returns a 1x3 matrix:
curl 'http://router.project-osrm.org/table/v1/driving/13.388860,52.517037;13.397634,52.529407;13.428555,52.523219;13.428555,52.523219?sources=0'
```

<img src="../references/pictures/osrm-matrix-1xn.png" alt="osrm-matrix-1xn" width="400"/>

One to many always be used if you have a single source(current location) and several potential destinations(gas stations), the result(ETA, distance) plus other information(price, waiting time) could help application do a better ranking.

### Many to many

```
# Returns a 3x3 matrix:
curl 'http://router.project-osrm.org/table/v1/driving/13.388860,52.517037;13.397634,52.529407;13.428555,52.523219;'
```
<img src="../references/pictures/osrm-matrix-nxm.png" alt="osrm-matrix-nxm" width="400"/>

The number of sources and destinations are multiplied to create the matrix or timetable.  For example, given location A, B, C, the API will return a matrix of all travel times between all locations.  

|Table|A    |B    |C    |
|------|-----|-----|-----|
|A     |A → A|A → B|A → C|
|B     |B → A|B → B|B → C|
|C     |C → A|C → B|C → C|

## Internal

### One to many


