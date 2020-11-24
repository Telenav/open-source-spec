# Static Graph
[Graphs](https://en.wikipedia.org/wiki/Graph_(abstract_data_type)) are used to represent many real-life applications. A widely-known example is to represent the Geographic information, then application is able to search [shortest paths](https://en.wikipedia.org/wiki/Shortest_path_problem) on the graph. This is also the basic idea of [OSRM](https://github.com/Project-OSRM/osrm-backend).      

**But how will you write code to represent a [graph](https://en.wikipedia.org/wiki/Graph_(abstract_data_type)) data structure?**    

[Adjacency list](https://en.wikipedia.org/wiki/Adjacency_list) and [Adjacency matrix](https://en.wikipedia.org/wiki/Adjacency_matrix) are most commonly used representations. [Adjacency list](https://en.wikipedia.org/wiki/Adjacency_list) are generally preferred because they efficiently represent [sparse graphs](https://en.wikipedia.org/wiki/Dense_graph). So let's make the question more concretely: **how will you write code to represent a [Adjacency list](https://en.wikipedia.org/wiki/Adjacency_list) based [graph](https://en.wikipedia.org/wiki/Graph_(abstract_data_type))?**     


Think about a simple graph sample(comes from [Graph and its representations](https://www.geeksforgeeks.org/graph-and-its-representations/)) as below, which has 5 vertices(a.k.a. nodes) and 7 edges(a.k.a. arcs) :    
```c++
0 - 1 \
| / |   2
4 - 3 /
```
Illustrated it as [Adjacency list](https://en.wikipedia.org/wiki/Adjacency_list) it may looks like this:    
![](https://cdncontribute.geeksforgeeks.org/wp-content/uploads/listadjacency.png)     

### Initial trying
The first version implementation may looks like this:     
```c++
// cpp
#include <iostream>
#include <string>
#include <list>

int main()
{
    // Adjacency list based graph
    std::list<int> graph[] = {
        {1, 4},
        {0, 4, 2, 3},
        {1, 3},
        {1, 4, 2},
        {3, 0, 1},
    };

    // print graph
    for (int i = 0; i < 5; i++){
        std::cout << i;
        for (auto v : graph[i]){
            std::cout << " -> " << v;
        }
        std::cout << std::endl;
    }
    return 0;
}

//output
0 -> 1 -> 4
1 -> 0 -> 4 -> 2 -> 3
2 -> 1 -> 3
3 -> 1 -> 4 -> 2
4 -> 3 -> 0 -> 1
```

It matches [Adjacency list](https://en.wikipedia.org/wiki/Adjacency_list) literally: uses an array to contains nodes, each node has an `list` to record its adjancency nodes.     

### Better one in `C++`
You may already aware that in `C++` we can use `std::vector` instead of `std::list` to avoid small pieces of memory allocation. Also, we'd better to replace `[]` by `std::vector` either. Now it will like this:    

```c++
// cpp
#include <iostream>
#include <string>
#include <vector>

int main()
{
    // Adjacency list based graph
    std::vector<std::vector<int>> graph = {
        {1, 4},
        {0, 4, 2, 3},
        {1, 3},
        {1, 4, 2},
        {3, 0, 1},
    };

    // print graph
    for (int i = 0; i < graph.size(); i++){
        std::cout << i;
        for (auto v : graph[i]){
            std::cout << " -> " << v;
        }
        std::cout << std::endl;
    }
    return 0;
}

// output
0 -> 1 -> 4
1 -> 0 -> 4 -> 2 -> 3
2 -> 1 -> 3
3 -> 1 -> 4 -> 2
4 -> 3 -> 0 -> 1
```
It looks much more practicable: simple but powerful. But we may still see bad performance if store a huge count nodes/edges, because there's still lots of small `std::vector`s in memory.        


## References
- [Graph (abstract data type)](https://en.wikipedia.org/wiki/Graph_(abstract_data_type))
- [Adjacency list](https://en.wikipedia.org/wiki/Adjacency_list)
- [Graph and its representations](https://www.geeksforgeeks.org/graph-and-its-representations/)
- [Graph theory](https://en.wikipedia.org/wiki/Graph_theory)
- [Shotest path algorithms](https://en.wikipedia.org/wiki/Shortest_path_problem#Algorithms)
- [sparse graphs](https://en.wikipedia.org/wiki/Dense_graph)
- [class StaticGraph](https://github.com/Project-OSRM/osrm-backend/blob/a3f1c2afb01ee23f23c88c1c3c78d31bbc88af48/include/util/static_graph.hpp#L119)

