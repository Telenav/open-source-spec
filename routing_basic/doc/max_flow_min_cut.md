- [Max Flow Min Cut](#max-flow-min-cut)
  - [Key points](#key-points)
    - [Residual network](#residual-network)
    - [Augmenting Path](#augmenting-path)
    - [Ford-Fulkerson](#ford-fulkerson)
    - [Max flow min cut theory](#max-flow-min-cut-theory)
    - [Worst case for FF](#worst-case-for-ff)
    - [Dinic](#dinic)
    - [Why level graph helps](#why-level-graph-helps)

# Max Flow Min Cut

Max Flow Min Cut is the algorithm be used in inertial flow to find a partition of a given graph.

For basic ideas, please go to Introduction to Algorithm.

## Key points

### Residual network

Flow network always be modeled as directed graph, we could only choose one direction for the flow between every pair of vertics.  But it is not clear in which direction the flow should go before solving the maxflow problem.  The direction be chosen during calculation could be wrong.  So we need ability to un-sent data back, represent on graph is by adding reverse edges and let algorithm has the ability to re-route back.

Residual network is consisted by edges with residual capacity.

###  Augmenting Path
When we try to find a maximum flow we are going to start with a suboptimal flow and the augment it with a flow we find in the residual network, which is called augmenting flow.
Augmenting path is also a valid flow in original network.

### Ford-Fulkerson

<img src="../resource/pictures/ford_fulkerson_alg.png" alt="ford_fulkerson_alg" width="400"/>

FF begin by initializing the flow to zero. Then, while there is a path from the source s to the sink t in the residual graph, it is going to calculate the minimum capacity along such a path and then augment with a flow that has this value along that path.
Once there are no more paths in the residual graph, we just return the current flow that we’ve found f.

FF will terminate in the range of max flow value.  Its time complexity is (num_of_flow * |E|)

[Implementation](https://github.com/CodeBear801/zoo/blob/master/graph/graph_partition/ford_fulkerson.cpp)

### Max flow min cut theory

Max cut is the cut of all flow from source to sink , which is the upper bound for the flow network

<img src="../resource/pictures/max_flow_min_cut_theory.png" alt="max_flow_min_cut_theory" width="400"/>
<br/>
<img src="../resource/pictures/max_flow_min_cut_pf.png" alt="max_flow_min_cut_pf" width="400"/>

For the upper graph, green vertex and orange vertex are two sets and there is no augmenting path could be found.  For the residual graph in the right, we could not push any data from green to orange, and for residual value from orange to green is unused for max flow.   We could conclude, any forward edge from green set to orange set is saturated and any backward edge is unused.

### Worst case for FF
<img src="../resource/pictures/worst_case_for_ff.png" alt="worst_case_for_ff" width="200"/>

Need augment for 200 times then find max flow

Obviously, it needs method to find better augmenting path.
  - Prefer heavier paths
  - Prefer shorter paths


### Dinic
Dinic try to find better augmenting path by shorter path.   Its key insight is that the work of computing the shortest path can be recycled so that a full precomputation only needs to happen when the shortest path distance changes, not for every augmenting path.

<img src="../resource/pictures/dinic_alg.png" alt="dinic_alg" width="400"/>

Dinic start with an initial flow of zero. Then repeat the following: build a level graph from the residual flow network, and let k be the length of a shortest path from s to t. Then while there is a path from source to sink that has this length k, which is be used to augment the flow and then update the residual capacities.
Then repeat this until there are no more s-t paths and we return the current flow.

[Implementation](https://github.com/CodeBear801/zoo/blob/master/graph/graph_partition/dinic.cpp)

### Why level graph helps
<img src="../resource/pictures/level_graph.png" alt="level_graph" width="400"/>

Level graph's creation based on the observation: augmenting along a shortest path only creates longer path.  Level graph could avoid the situation of route back and forth, once exhausted all paths of a given length, the next shortest path must be one edge longer




