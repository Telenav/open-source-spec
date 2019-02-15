# OSRM Partition Detail

## Partition Input
The input for partition step is cnbg_to_ebg and ebg. <br/>
cnbg_to_ebg maps node based graph to edge based graph.  It records edges in the format as
```C++
struct CompressedNodeBasedGraphEdge
{
    NodeID source;
    NodeID target;
};
```
ebg contains uncontracted edge-based graph, indexed by EBG NodeID
```C++
struct Coordinate
{
    FixedLongitude lon;
    FixedLatitude lat;
}
```

More context could be found here [code](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/partitioner.cpp#L45)


## Generate Graph
 BisectionGraph([type](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/partitioner/bisection_graph.hpp#L56), [interface](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/partitioner/partition_graph.hpp#L64)) is the data structure used for graph partition step.  [makeBisectionGraph](end/blob/v5.20.0/include/partitioner/bisection_graph.hpp#L60) is the function used to construct which, it reorder edges and build adjencent list

 ```C++
inline BisectionGraph makeBisectionGraph(const std::vector<util::Coordinate> &coordinates,
                                         const std::vector<BisectionInputEdge> &edges)
 {
//...
   // create a bisection node, requires the ID of the node as well as the lower bound to its edges
    const auto make_bisection_node = [&edges, &coordinates](
        const std::size_t node_id, const auto begin_itr, const auto end_itr) {
        std::size_t range_begin = std::distance(edges.begin(), begin_itr);
        std::size_t range_end = std::distance(edges.begin(), end_itr);
        return BisectionGraph::NodeT(range_begin, range_end, coordinates[node_id], node_id);
    };
//...

 }

 ```
The definition of two important data structure worth you to take a look: BisectionInputEdge and BisectionGraphNode.


## Bisection Graph
Most of partition logic happened in the constructor of [RecursiveBisection](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/recursive_bisection.cpp#L33).  The use of Intel-thread-building-blocks could speed up these process dramatically.

### Find strong connected components
[PrePartitionWithSCC](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/recursive_bisection_state.cpp#L91) is the function calculate strong connected component, and you could find the implementation of algorithm [here](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/extractor/tarjan_scc.hpp#L62)

### Find most suitable bisection
[Inertial flow] is the algorithm used for graph partition, which is the most important part of CRP and MLD.
// to do


## Convert bisection result to partition


## Remove unconnected boundary edges


## Renumber graph


## Output

