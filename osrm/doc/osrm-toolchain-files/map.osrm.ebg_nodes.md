# .osrm.ebg_nodes
Contains edge-based graph nodes and annotations.      


## List

```bash
tar -tvf nevada-latest.osrm.ebg_nodes
-rw-rw-r-- 0/0               8 1970-01-01 00:00 osrm_fingerprint.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/ebg_node_data/nodes.meta
-rw-rw-r-- 0/0         8744964 1970-01-01 00:00 /common/ebg_node_data/nodes
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/ebg_node_data/annotations.meta
-rw-rw-r-- 0/0          421296 1970-01-01 00:00 /common/ebg_node_data/annotations
```

## osrm_fingerprint.meta
- [osrm_fingerprint.meta](./fingerprint.md)

## /common/ebg_node_data/nodes, /common/ebg_node_data/nodes.meta
Contains edge based nodes.      

### Layout
![](./graph/map.osrm.ebg_nodes.common.ebg_node_data.nodes.png)

### Implementation


## /common/ebg_node_data/annotations, /common/ebg_node_data/annotations.meta
Stores annotation data that will be referenced by `edge_based_node`.      
It's almost the same with the annotation data for `node_based_edge`. The only difference is that the data here have been compressed(remove unused and duplicated) by [NodeBasedGraphFactory::CompressAnnotationData()](https://github.com/Telenav/osrm-backend/blob/038ddf0f72df7c55aa51f3d1d201289347007c36/src/extractor/node_based_graph_factory.cpp#L146).      

### Layout & Implementation
See more in [/extractor/annotations in .osrm](https://github.com/Telenav/open-source-spec/blob/master/osrm/doc/osrm-toolchain-files/map.osrm.md#extractorannotations-extractorannotationsmeta).     


