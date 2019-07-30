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

More context could be found [here](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/partitioner.cpp#L45)


## Generate Graph
 BisectionGraph([type](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/partitioner/bisection_graph.hpp#L56), [interface](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/partitioner/partition_graph.hpp#L64)) is the data structure used for graph partition step.  [makeBisectionGraph](https://github.com/Project-OSRM/osrm-backend/blob/9234b2ae76bdbbb91cbb51142bfc0ee1252c4abd/include/partitioner/bisection_graph.hpp#L60) is the function used to construct which, it reorder edges and build adjacent list

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


## Bisec Graph
Most of partition logic happened in the constructor of [RecursiveBisection](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/recursive_bisection.cpp#L33).  The use of Intel-thread-building-blocks could speed up these process dramatically.
```C++
    // Parallelize recursive bisection trees. Root cut happens serially (well, this is a lie:
    // since we handle big components in parallel, too. But we don't know this and
    // don't have to. TBB's scheduler handles nested parallelism just fine).
    //
    //     [   |   ]
    //      /     \         root cut
    //  [ | ]     [ | ]
    //  /   \     /   \     descend, do cuts in parallel
    //
    // https://www.threadingbuildingblocks.org/docs/help/index.htm#reference/algorithms/parallel_do_func.html

    // [Perry] TreeNode likes bisection tasks, until reach stop condition it will continue to split and add to the tree
    struct TreeNode
    {
        BisectionGraphView graph;
        std::uint64_t depth;
    };
    
    // Bisect graph into two parts. Get partition point and recurse left and right in parallel.
    tbb::parallel_do(begin(forest), end(forest), [&](const TreeNode &node, Feeder &feeder) {
        // ...
    }
```



### Find strong connected components
[PrePartitionWithSCC](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/recursive_bisection_state.cpp#L91) is the function calculate strong connected component, and you could find the implementation of algorithm [here](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/extractor/tarjan_scc.hpp#L62)<br/>

### Find most suitable bisection
[Inertial flow]() is the algorithm used for graph partition, which is the most important part of [CRP](../../../routing_basic/doc/crp.md) and [MLD](../bidirectional_dijkstra_in_osrm.md).  [This blog](https://daniel-j-h.github.io/post/selection-algorithms-for-partitioning/) describe the logic pretty clear, and its coming from author for this part's code. <br/>

```C++
// entrance point
DinicMaxFlow::MinCut computeInertialFlowCut(const BisectionGraphView &view,
                                            const std::size_t num_slopes,
                                            const double balance,
                                            const double source_sink_rate)
//...

// main logic happened in this function
DinicMaxFlow::MinCut bestMinCut(const BisectionGraphView &view,
                                const std::size_t n,
                                const double ratio,
                                const double balance)
```

bestMinCut will try with different set of source/sink nodes and find best cut on each of them, then pick the one with minimum cost for bestMinCut.<br/>
For selecting different source/sink nodes, you could go to function [makeSpatialOrder](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/inertial_flow.cpp#L39), and function [reorderFirstLast](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/partitioner/reorder_first_last.hpp#L21) is the one you should not missed.<br/>
After source/sink nodes be selected, OSRM use [Dinic algorithm](https://en.wikipedia.org/wiki/Dinic%27s_algorithm) to calculate [max flow min cut](../../../routing_basic/doc/max_flow_min_cut.md).  Implementation is [here](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/dinic_max_flow.cpp#L39) and you could follow the steps of function listed below<br/>
```C++
    DinicMaxFlow::ComputeLevelGraph

    DinicMaxFlow::BlockingFlow

    DinicMaxFlow::GetAugmentingPath

    DinicMaxFlow::MakeCut

```


## Convert bisection result to partition
Logic can be found in function [bisectionToPartition](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/bisection_to_partition.cpp#L117).<br/>

## Remove unconnected boundary edges
Logic can be found in function [removeUnconnectedBoundaryNodes](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/include/partitioner/remove_unconnected.hpp).<br/>

For example, 
```C++
    /*
    0---1  4---5  8---7 
     \ /    \ /    \ /
      2      3------6
    */
```
If the partition algorithm put {0, 1, 2, 3} in one partition and {4, 5, 6, 7, 8} in another partition, this function will dectect that and make partition as {3, 4, 5, 6, 7, 8}<br/>



## Renumber graph
Logic can be found in function [makePermutation](https://github.com/Project-OSRM/osrm-backend/blob/v5.20.0/src/partitioner/renumber.cpp#L44)<br/>


## Output

