- [QueryHeap In OSRM](#queryheap-in-osrm)
  - [QueryHeap In OSRM](#queryheap-in-osrm-1)
  - [std::priority_queue vs. boost::heap::d_ary_heap](#stdpriorityqueue-vs-boostheapdaryheap)
    - [TODO](#todo)
  - [References](#references)

# QueryHeap In OSRM

## QueryHeap In OSRM 

| Action Interfaces | QueryHeap In OSRM |
|-------------|---------------------|
|Insert|`Insert()`|
|Find | `WasInserted()` |
|Pop | `DeleteMin()` |

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
[why 4-ary heap performs better than 2-ary heap for decrease() and extract-min()](https://stackoverflow.com/questions/29126428/binary-heaps-vs-d-ary-heaps/31015405#31015405)


### TODO
- there's another choice [Fibonacci Heap](https://en.wikipedia.org/wiki/Fibonacci_heap) may lead to better performance for [Dijkstra](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm), worthing to have a try.  Time complexity is `O(M + Nlog(N)`(with N =|V| and M = |A|)
- If arc lengths are integers bounded by C, [multilevel buckets](https://pubsonline.informs.org/doi/abs/10.1287/opre.27.1.161) could achieve `O(M + nlogC/loglogC)`
   
## References
- [class QueryHeap](https://github.com/Project-OSRM/osrm-backend/blob/72e03f9af9824cbb1d26cba878f242eb0feae584/include/util/query_heap.hpp#L195)
- [d-ary heap](https://en.wikipedia.org/wiki/D-ary_heap)
- [Heap](https://en.wikipedia.org/wiki/Heap_(data_structure))
- [Heapsort](https://en.wikipedia.org/wiki/Heapsort)
- [Priority Queue](https://en.wikipedia.org/wiki/Priority_queue)
- [boost::heap::d_ary_heap](https://www.boost.org/doc/libs/1_66_0/doc/html/boost/heap/d_ary_heap.html)
- [std::priority_queue](http://www.cplusplus.com/reference/queue/priority_queue/)  
