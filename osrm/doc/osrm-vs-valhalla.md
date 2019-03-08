<!-- TOC -->
- [OSRM vs Valhalla](#osrm-vs-valhalla)
  - [From architect's perspective](#from-architects-perspective)
  - [From programmer's perspective](#from-programmers-perspective)
  - [From product manager's perspective](#from-product-managers-perspective)
  - [From QA's perspective](#from-qas-perspective)

# OSRM vs Valhalla

[**OSRM**](../README.md) and [**Valhalla**](../../valhalla/README.md) are two popular open source routing engines.  Both of them have worldwide contributors, majority developer for OSRM is MapBox and Lyft, for Valhalla is MapZen and Tesla.  <br/>

MapBox, Pinterest and Foursquare chosed OSRM for cloud routing.  Tesla selects Valhalla as its navigation engine. <br/>

This page tracks highlights from both engines, we also try to list design tradeoffs so it would be easier to find best practise. <br/>

**One Line Summary**
- OSRM: accurate route with live traffic, monolithic data, high memory needs, very high performance
- Valhalla: flexible tiled data, customizable routing, low memory requirements, reasonably fast point-to-point routing

## From architect's perspective

**Direction as service** <br/>
Hide all logic such as algorithm/version control/traffic in the service itself.  Both of them use node.js to provide restful request and response.<br/>

**Release Docker**<br/>
Docker image hides the complexity of dependency management and deployment complexity<br/>

**Separate algorithm with dynamic configuration**<br/>
For data import and cost model, both of the engine use Lua to manage frequently change items<br/>



## From programmer's perspective

**Algorithm**<br/>
OSRM provides an example of modern cloud solution.<br/>
Its routing solution is based on [**CRP**](../../routing_basic/doc/crp.md), which has been widely used by cloud based navigation products.  CRP considers all elements during real time route calculation which provides a firm foundation for cloud routing application, while all previous Telenav cloud routing solution need to have trade-off between quality and performance during handling real time information.<br/> 
OSRM's MapMatching solution is based on Hidden Markov Model and Viterbi, which also proven to be standard and best practice.  The same strategy also be used in [**Valhalla Map Matching**](../../valhalla/doc/valhalla-mapmatching.md).  <br/>

**Data structure**<br/>
To hide complexity of geographic road graph information, OSRM defines [**graph representation**](./understanding_osrm_graph_representation.md) which decouples graph algorithm with cost model <br/>

**Data Schema**<br/>
Valhalla gave us an example of how to design data schema for Streaming<br/>
[**Fixed tile's boundary**](../../valhalla/doc/valhalla-tile-general.md) in Valhalla makes its easy to enable streaming feature, like caching home area data, onboard routing and guidance.<br/>
Valhalla could calculate route for multiple mode, such as pedestrian, bike, public transportation. <br/>

**Parallel processing**<br/>
Both of them use [**OSMIUM**](../../osmium/README.md) and [Intel-ThreadBuildingBlocks](https://www.threadingbuildingblocks.org/) to speed up processing.  OSMIUM helps to divide and conquer large input files, ThreadBuildingBlocks act as MapReduce in single process.  <br/>


## From product manager's perspective


||OSRM|Valhalla|
|----------------|--------------------------|--------------------------|
|[Summary by Kevin Kreiser](https://github.com/valhalla/valhalla/issues/1514#issuecomment-419160356)|1. It precomputes routes so requests are super fast (faster than valhalla)<br/>2. The precomputed stuff is stored in memory so depending on your dataset you might need a lot of ram<br/>3. Precomputing them means you can't get different routes at runtime (ie. avoid tolls, favor hills, penalize ferries, etc) <br/>because its precomputed it will only support one mode of travel<br/>4. Localization support is better although comes via a javascript add-on|1. It only stores the graph so routes are computed on the fly (slower than OSRM)<br/> 2. It doesn't store the entire graph in memory so you can run it on memory constrained devices<br/>3. You can get different routes at run time by varying some parameters in the request<br/>4. Since it stores the graph you can change mode of travel on the fly (bike/ped/car/motorcycle/truck/etc)<br/>5. Localization is built into the engine but not as extensive as OSRMTI<br/>6. The data is tiled so you can make the planet and then distribute smaller extracts<br/>7. Supports time based routing<br/>8. Supports transit routing<br/>9. Supports isochrones|
|Pros<br/>(Feature could learn)|Support display-tile service<br/>Support multi-mode travel|Support streaming feature is its original target<br/>Fixed tile boundary<br/>Road data is not duplicated between levels|
|Cons<br/>(Need improve)|Additional effort for HERE/TomTom support<br/>Complex restriction and condition oneway is not supported<br/>TomTom Traffic source support<br/>Large memory usage<br/>|Not much of data compression strategy([profile](../../valhalla/doc/valhalla-tile-profile.md))<br/>Only one thread for initilization<br/>Lack of code orgnization for forward/backward compatibility|

## From QA's perspective

**Use business-readable specifications to test product features**<br/>
Both of engines provide sufficient unit test to guarantee code quality, and also implement javascript framework to visualize routing test result, but most impressive part is the use of [Cucumber testing framework](https://cucumber.io/) in OSRM.  It enables product owner or QA to use stories to write testing cases, which guides final result is really the expected one.  You could find samples [here](https://github.com/Project-OSRM/osrm-backend/blob/master/features/foot/oneway.feature).



