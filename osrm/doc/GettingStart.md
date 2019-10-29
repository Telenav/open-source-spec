<!-- TOC -->

- [Getting Start with OSRM](#getting-start-with-osrm)
    - [What is OSRM?](#what-is-osrm)
    - [Building from Source and Debugging](#building-from-source-and-debugging)
        - [Building](#building)
        - [Generate Xcode Project](#generate-xcode-project)
        - [Run and Debug](#run-and-debug)
            - [Debug in Xcode](#debug-in-xcode)
            - [Download OSM data](#download-osm-data)
        - [If you still want to debug on native Windows](#if-you-still-want-to-debug-on-native-windows)
    - [Reading Code](#reading-code)
        - [osrm-routed](#osrm-routed)

<!-- /TOC -->

# Getting Start with OSRM 
I met a lot of issues when I started the OSRM learning/research, so I built this document to help newcomers to avoid them.      
Moreover, also some experiences recoreded.       
Welcome everyone to make this document up to date!    
- Based on revision: `commit e5d8319c43b454642079f66b930ac73803840e2b`    

## What is OSRM? 
I assume you already know basically what OSRM is.    
If not, please start from :    
- read the [main page of osrm-backend](https://github.com/Project-OSRM/osrm-backend)    
- try the [demo server of OSRM](http://map.project-osrm.org/)    
- quick view the [OSRM API Documentation](http://project-osrm.org/)    

There're totally 2 main repos for OSRM project, i.e. [osrm-backend](https://github.com/Project-OSRM/osrm-backend) and [osrm-frontend](https://github.com/Project-OSRM/osrm-frontend). What we will focus on is the [osrm-backend](https://github.com/Project-OSRM/osrm-backend) repo, i.e. the routing engine.       

OSRM is designed to run on [OpenStreetMap](https://www.openstreetmap.org/) data. For a quick introduction about how the road network is represented in OpenStreetMap and how to map specific road network features have a look at [this guide about mapping for navigation](https://www.mapbox.com/mapping/mapping-for-navigation/).    

There're also a lot of documents on [osrm-backend wiki page](https://github.com/Project-OSRM/osrm-backend/wiki) which are very valuable for a quick view.    

## Building from Source and Debugging    
Even OSRM provides docker image to help us build our own OSRM service easier, building and debugging the source code can still help us to understand the project deeply.    

### Build 
Before you start, please     
- use a Mac    
    - Actually OSRM can be built and run on many platforms, e.g. MacOSX, kinds of Linux, even native Windows (may not work, see [Windows Compilation](https://github.com/Project-OSRM/osrm-backend/wiki/Windows-Compilation)). But for debug the project, it's better to have a Mac with Xcode.    
- get familiar with modern C++ (C++11/14)    
- install pre-requirements: latest `git, cmake`    

Refer to below links for other dependencies and detailed build steps:    
- [Build from Source](https://github.com/Project-OSRM/osrm-backend#building-from-source)    
- [Building-OSRM](https://github.com/Project-OSRM/osrm-backend/wiki/Building-OSRM)    

### Generate Xcode Project    
The current default build steps on MacOSX will not generate Xcode project. Also, try to generate Xcode project by `cmake -G Xcode` command can not work(see the still opened issue [How to create Xcode project of osrm-backend on mac](https://github.com/Project-OSRM/osrm-backend/issues/2409)).      
Fortunately @springmeyer have found the reason and give a temp workaround for us(please also refer to the answer from @springmeyer in [How to create Xcode project of osrm-backend on mac](https://github.com/Project-OSRM/osrm-backend/issues/2409)):   
1. Go to local cloned `osrm-backend` repository.
2. `touch src/dummy.cpp`<br>
3. Apply a patch for CMakeLists.txt.<br>
3.1 get [patch file](../references/files/enable-xcode-project.diff) and put it in current folder<br>
3.2 `git apply enable-xcode-project.diff`<br>
3.3 If the apply fails in the future when the original CMakeLists file has some extra changes, then you could manually add the diffs into it. <br>
3.4 The idea is simple, somehow the targets `libosrm_store.a` and `libosrm_updater.a` can't be generated correctly, which makes linking can't be finished correctly. So we need to make a workaround to generate the targets.<br>
4. Then run the following commands: 
```
mkdir build
cd build
```
5. Then you have two choices:
```
Build with Mason package manager (Recommended).

cmake ../ -G Xcode -DENABLE_MASON=ON -DBUILD_TOOLS=1
cmake --build .
```
```
Manually install all the dependencies. (You may need to install some specific one if Mason manager can't install if for you in the first way.)

brew install boost git cmake libzip libstxxl libxml2 lua tbb ccache GDAL (only select what you don't have)
cmake .. -G Xcode
cmake --build .
```
6. You will find `OSRM.xcodeproj` under `build` folder.

### Run and Debug   
There're several runable binaries, i.e. `osrm-components, osrm-contract, osrm-customize, osrm-datastore, osrm-extract, osrm-io-benchmark, osrm-partition, osrm-routed`, most of them are used for pre-processing map data.    
- `osrm-extract`: Extract the road network(routing data) into a normalized format. It parses the contents of the exported OSM file and writes out routing metadata. More detailed refer to [Extracting the Road Network (osrm-extract)](https://github.com/Project-OSRM/osrm-backend/wiki/Running-OSRM#extracting-the-road-network-osrm-extract).    
- For the MLD(Multi-Level Dijkstra) pipeline we need to extract (`osrm-extract`) a graph out of the OpenStreetMap base map, then partition (`osrm-partition`) this graph recursively into cells, customize the cells (`osrm-customize`) by calculating routing weights for all cells.    
    - i.e. MLD data pre-processing pipeline: `osrm-extract`->`osrm-partition`->`osrm-customize`    
- For CH(Contraction Hierarchies) the partition and customize pipeline stage gets replaced by adding shortcuts from the Contraction Hierarchies algorithm (`osrm-contract`).   
    - i.e. CH data pre-processing pipeline: `osrm-extract`->`osrm-contract`    

Please have a try by the [Run the routing engine](https://github.com/Project-OSRM/osrm-backend/wiki/Running-OSRM)    

#### Debug in Xcode    
- Open the Xcode project    
- choose the target you wanted(e.g. `osrm-routed`)    
- set the command line parameter: `Edit Scheme -> Run -> Arguments -> Arguments Passed On Launch`    
- Run    
- Enjoy!     

#### Download OSM data    
Exported OSM data files can be obtained from providers such as [Geofabrik](http://download.geofabrik.de/).     
I have tried to download the `.osm.pbf` file from [Geofabrik](http://download.geofabrik.de/), it works!    

### If you still want to debug on native Windows    
I also tried this on my Windows 10, so far it works.     
- Build with VS2015    
    - There're some dependencies will be downloaded while run this `build-local.bat`. Some of them you may already have and want to avoid the redundant download, you can try to edit the `build-local.bat` and `appveyor-build.bat`.    
        - e.g. If already have `cmake`, change the `SET CMAKE_VERSION=3.9.2` with your current cmake version.    
```
cd osrm-backend
./build-local.bat 
```    
- Open Visual Studio 2015 Solution: `./build/OSRM.sln`    
- Set Configuration to `RelWithDebInfo` (`Debug` build failed)    
- Set `osrm-routed` as Startup Project    
- Enjoy!    

## Reading Code   
The document [OSRM Processing Flow](https://github.com/Project-OSRM/osrm-backend/wiki/Processing-Flow) is a description from importing raw OSM data, to computing routes and serving requests. Very valuable for reference.
[Graph representation](https://github.com/Project-OSRM/osrm-backend/wiki/Graph-representation) describe basic graph structure of OSRM.

### osrm-routed   
This is a demo HTTP server on top of `libosrm` library, the core routing service engine. More detailed refer to [Running the Engine via a HTTP Server (osrm-routed)](https://github.com/Project-OSRM/osrm-backend/wiki/Running-OSRM#running-the-engine-via-a-http-server-osrm-routed).    

