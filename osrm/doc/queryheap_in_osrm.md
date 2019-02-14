# QueryHeap - PriorityQueue and VisitedTable In OSRM
[OSRM](https://github.com/Project-OSRM/osrm-backend) uses a single data structure [QueryHeap](https://github.com/Project-OSRM/osrm-backend/blob/72e03f9af9824cbb1d26cba878f242eb0feae584/include/util/query_heap.hpp#L195) to represent both what we call `priority_queue` and `visited_table`. The idea of [QueryHeap](https://github.com/Project-OSRM/osrm-backend/blob/72e03f9af9824cbb1d26cba878f242eb0feae584/include/util/query_heap.hpp#L195) is similar with `class SearchGraph` of `routingcore`, but different implementation.    

## QueryHeap In OSRM vs. SearchGraph In routingcore
### Action Interfaces
| Action Interfaces | QueryHeap In OSRM | SearchGraph In routingcore |
|-------------|---------------------|--------------------------------|
|Insert|`Insert()`|`PQ_Add()`/`Hashmap_Add()`|
|Find | `WasInserted()` | `Hashmap_Find()` |
|Pop | `DeleteMin()` | `PQ_Pop()` |

### Similar Members Between QueryHeap And SearchGraph
There're 3 members in [QueryHeap](https://github.com/Project-OSRM/osrm-backend/blob/72e03f9af9824cbb1d26cba878f242eb0feae584/include/util/query_heap.hpp#L195):     
```c++
    // [Jay] inserted_nodes will store one HeapNode during each `Insert()` action. 
    //  The insert location (always at the end) will be the internal_index for each Node. 
    //  similar with: `ReuseableVector<RouteItem, 15> m_visitedTable` of `SearchGraph`
    std::vector<HeapNode> inserted_nodes;

    // [Jay] Priority Queue (boost::heap::d_ary_heap actually)
    //  similar with: `std::priority_queue<QUEUE_ITEM> m_qRoute` of `SearchGraph`
    HeapContainer heap;

    // [Jay] NodeID -> internal_index Map
    //  Default will be an array storage, but most of time it will be a `std::unordered_map`
    //  similar with: `HASHMAP<uint64_t, uint32_t> m_mSegmentId2RelaxIdx` of `SearchGraph`
    IndexStorage node_index;
```     
The `Insert()` operation can tell us how it works deeply:    
```c++
    void Insert(NodeID node, Weight weight, const Data &data)
    {
        BOOST_ASSERT(node < std::numeric_limits<NodeID>::max());
        const auto index = static_cast<Key>(inserted_nodes.size());
        const auto handle = heap.push(std::make_pair(weight, index));
        inserted_nodes.emplace_back(HeapNode{handle, node, weight, data});
        node_index[node] = index;
    }

```
### Different Member Between QueryHeap And SearchGraph
There still has another key member `HASHMAP<uint32_t, uint16_t> m_mEdge2Shadow` in `SearchGraph`, to handle the case enter the same edge from different source paths.     
- Why it necessary in our code is:   
    - When we settled an edge for more than 1 time, we recorded all of them with different costs, the smallest cost will be used first by `PriorityQueue`.     
    - But when it's un-navigable(e.g. meet restriction), we have to prune it, then pick up next one for relax.    
- How OSRM solve this problem?        
    - OSRM has already handled restriction during pre-processing, so that only necessary to focus on the minimum cost during Query. (Refer to [Understanding OSRM Graph Representation](./understanding_osrm_graph_representation.md) for more details.)    
    - If meet newer path to an same edge with smaller cost, just update previous one in `QueryHeap`.    



## std::priority_queue vs. boost::heap::d_ary_heap
[Dijkstra](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) based algorithms require a `Priority Queue` structure to iterate by ascending ordered weight(i.e. cost). Most of implementations will use the standard one: `std::priority_queue`, but [OSRM](https://github.com/Project-OSRM/osrm-backend) use `boost::heap::d_ary_heap`(`4-ary`) instead.    

- [std::priority_queue](http://www.cplusplus.com/reference/queue/priority_queue/)    
`std::priority_queue` implemented based on [std::make_heap](http://www.cplusplus.com/reference/algorithm/make_heap/), [std::push_heap](http://www.cplusplus.com/reference/algorithm/push_heap/) and [std::pop_heap](http://www.cplusplus.com/reference/algorithm/pop_heap/) for [Heap](https://en.wikipedia.org/wiki/Heap_(data_structure)) properties. It's actually a [binary heap](https://en.wikipedia.org/wiki/Binary_heap) in STL implemtation.     
Since our Bidirectional A-Star Algorithm in `routingcore` will only need `insert` and `pop_min` operations, the most important for us is the algorithm complexity of [std::push_heap](http://www.cplusplus.com/reference/algorithm/push_heap/) and [std::pop_heap](http://www.cplusplus.com/reference/algorithm/pop_heap/).    
    - `std::push_heap`: `O(log(N))`
    - `std::pop_heap` : `O(log(N))`

- [boost::heap::d_ary_heap](https://www.boost.org/doc/libs/1_66_0/doc/html/boost/heap/d_ary_heap.html)    
[boost::heap::d_ary_heap](https://www.boost.org/doc/libs/1_66_0/doc/html/boost/heap/d_ary_heap.html) is a implementation of [d-ary heap](https://en.wikipedia.org/wiki/D-ary_heap).      
    - Below refers to wikipedia of [d-ary heap](https://en.wikipedia.org/wiki/D-ary_heap):    
The d-ary heap or d-heap is a priority queue data structure, a generalization of the binary heap in which the nodes have d children instead of 2. Thus, a binary heap is a 2-heap, and a ternary heap is a 3-heap.    
This data structure allows decrease priority operations to be performed more quickly than binary heaps, at the expense of slower delete minimum operations. 4-heaps may perform better than binary heaps in practice, even for delete-min operations.    
    - algorithm complexity of `insert` and `pop_min` operations of `boost::heap::d_ary_heap`:    
        - `push`: `O(log(N)/log(d))`
        - `pop`: `O(log(N)/log(d))`

As above, `4-ary heap` of `QueryHeap` is very similar with `binary heap` of `std::priority_queue`, the only difference between them is `4-ary` vs `2-ary`. Both `push` and `pop` of them have logarithmic algorithm complexity.  


### TODO
- have a try with `4-ary heap` in `routingcore`
- there's another choice [Fibonacci Heap](https://en.wikipedia.org/wiki/Fibonacci_heap) may lead to better performance for [Dijkstra](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm), worthing to have a try.  Time complexity is `O(M + Nlog(N)`(with N =|V| and M = |A|)
- If arc lengths are integers bounded by C, [multilevel buckets](https://pubsonline.informs.org/doi/abs/10.1287/opre.27.1.161) could achieve `O(M + nlogC/loglogC)`
   
## References
- [class QueryHeap](https://github.com/Project-OSRM/osrm-backend/blob/72e03f9af9824cbb1d26cba878f242eb0feae584/include/util/query_heap.hpp#L195)
- [class SearchGraph](https://bitbucket.telenav.com/projects/NAV/repos/navcore/browse/routingcore/src/Direction/Routing/route_search_graph.h)
- [d-ary heap](https://en.wikipedia.org/wiki/D-ary_heap)
- [Heap](https://en.wikipedia.org/wiki/Heap_(data_structure))
- [Heapsort](https://en.wikipedia.org/wiki/Heapsort)
- [Priority Queue](https://en.wikipedia.org/wiki/Priority_queue)
- [boost::heap::d_ary_heap](https://www.boost.org/doc/libs/1_66_0/doc/html/boost/heap/d_ary_heap.html)
- [std::priority_queue](http://www.cplusplus.com/reference/queue/priority_queue/)  