<!-- TOC -->

- [.osrm.geometry](#osrmgeometry)
    - [List](#list)
    - [osrm_fingerprint.meta](#osrm_fingerprintmeta)
    - [/common/segment_data/*](#commonsegment_data)
        - [Layout](#layout)
        - [Implementation](#implementation)

<!-- /TOC -->

# .osrm.geometry
It contains geometry that was removed from the original graph. Used for returned API response.    

## List

```bash
tar -tvf nevada-latest.osrm.geometry
-rw-rw-r-- 0/0               8 1970-01-01 00:00 osrm_fingerprint.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/index.meta
-rw-rw-r-- 0/0         1566256 1970-01-01 00:00 /common/segment_data/index
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/nodes.meta
-rw-rw-r-- 0/0         6831608 1970-01-01 00:00 /common/segment_data/nodes
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/forward_weights/number_of_elements.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/forward_weights/packed.meta
-rw-rw-r-- 0/0         4696736 1970-01-01 00:00 /common/segment_data/forward_weights/packed
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/reverse_weights/number_of_elements.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/reverse_weights/packed.meta
-rw-rw-r-- 0/0         4696736 1970-01-01 00:00 /common/segment_data/reverse_weights/packed
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/forward_durations/number_of_elements.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/forward_durations/packed.meta
-rw-rw-r-- 0/0         4696736 1970-01-01 00:00 /common/segment_data/forward_durations/packed
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/reverse_durations/number_of_elements.meta
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/reverse_durations/packed.meta
-rw-rw-r-- 0/0         4696736 1970-01-01 00:00 /common/segment_data/reverse_durations/packed
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/forward_data_sources.meta
-rw-rw-r-- 0/0         1707902 1970-01-01 00:00 /common/segment_data/forward_data_sources
-rw-rw-r-- 0/0               8 1970-01-01 00:00 /common/segment_data/reverse_data_sources.meta
-rw-rw-r-- 0/0         1707902 1970-01-01 00:00 /common/segment_data/reverse_data_sources
```

## osrm_fingerprint.meta
- [osrm_fingerprint.meta](./fingerprint.md)

## /common/segment_data/*

### Layout

### Implementation


