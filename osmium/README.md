# OSMIUM in Telenav    

OSMIUM is a fast and flexible C++ toolkit and framework for working with OSM data.
Here is the [slides](./resource/slides/osmium_20181201.pptx) records discussion in US Nav team and CN data content team.
OSMIUM has been used widely in OSM world and Telenav internal projects, all of the users benefit from its elegant and efficient design.

## Problem set

### 1. When you have 40 billion of roads, how do you design efficient data schema to hold intermediate data
 
       - OSM PBF format
       - OSM primitive
       - PBF VarInt(types.hpp), string_table(this is the definition in osm pbf)
 

### 2. When you try to explore extremely large data(over 100GB), how to design proper structure to process

       - Separate reading thread vs processing thread
       - General idea is one specific thread focus on reading data as byte string, record into thread safe queue
       - Use std::future to communicate between thread
 

### 3. How to make full use of your hardware to maximize processing speed

      - Treat decoding blobs as tasks
      - Thread pool to decode OSM object
 

### 4. How to reduce memory allocation and deallocation during running time

      - Buffer as memory pool, serialize osm objects on buffer
      - Iterator to visit data based on buffer



## Things to Learn
- The pattern of reader->processor->handler
- C++11's style to implement or use thread-safe queue, future, threadpool, buffer
- Divide and conquer



## Reference
- [osmium wiki](https://wiki.openstreetmap.org/wiki/Osmium)
- [osmium github](https://github.com/osmcode/libosmium)
- [talk from author](./resource/slides/Osmium-to-the-Rescue.pdf)
- [osm pbf decoder in golang](./doc/pbf_golang.md)
