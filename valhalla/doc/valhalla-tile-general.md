# Valhalla Tile

Tile is the unit to record navigation data and supporting Routing/Guidance/MapMatching service.  Valhalla choose same size of grid for tile's unit.

<img src="../resource/pictures/world_level0.png" alt="single_cell" width="400"/>


## Important ideas

### 1. Fixed boundary
Fixed boundary provide a stable mapping between geometry information and database files, which is suitable for streaming cache.  More information, please go to here: [valhalla-tiles-basic](./valhalla-tiles-basic.md)


### 2. Use mmap to SpeedUp
To speed up file loading, Valhalla using mmap to load flat package with uncompressed files.  More information, please go to here: [valhalla-tile-read-logic](./valhalla-tile-read-logic.md).

### 3. No line clipping at boarder
Valhalla don't support map rendering service, there is no logic to handle polygon clipping at tile boarder.

