# Table service

## API
Table service also be called [Matrix API](https://docs.mapbox.com/help/how-mapbox-works/directions/#mapbox-matrix-api), which is used for calculated travel times/distance between many locations.  

## Examples
Based on the definition in [OSRM's table-service API Documentation](http://project-osrm.org/docs/v5.5.1/api/#table-service), we construct following examples:

### One to many

```
# Returns a 1x3 matrix:
curl 'http://router.project-osrm.org/table/v1/driving/13.388860,52.517037;13.397634,52.529407;13.428555,52.523219;13.428555,52.523219?sources=0'
```

<img src="../references/pictures/osrm-matrix-1xn.png" alt="osrm-matrix-1xn" width="400"/>

One to many always be used if you have a single source(current location) and several potential destinations(gas stations), the result(ETA, distance) plus other information(price, waiting time) could help application do a better ranking.

### Many to many

```
# Returns a 3x3 matrix:
curl 'http://router.project-osrm.org/table/v1/driving/13.388860,52.517037;13.397634,52.529407;13.428555,52.523219;13.428555,52.523219;13.447496,52.524255?sources=0;1&destinations=2;3;4'
```
<img src="../references/pictures/osrm-matrix-nxm.png" alt="osrm-matrix-nxm" width="400"/>

The number of sources and destinations are multiplied to create the matrix or timetable.  For example, given location A, B, C, D, E and define A/B as sources and C/D/E as destinations the API will return a matrix of all travel times between {A, B} and {C, D, E}.  

|Table|A    |B    |
|------|-----|-----|
|C     |A → C|B → C|
|D     |A → D|B → D|
|E     |A → E|B → E|

## Internal

### One to many
One to many's logic likes single direction dijkstra exploration, start from source and then expand out, after all destination candidates has been met then program will stop.

#### One to many(mld)


##### Speed up by MLD

###### Example


<img src="../references/pictures/mld_routing_example_graph_partition.png" alt="mld_routing_example_graph_partition.png" width="800"/>

Let's say we want to calculate a route form source=node_0 to destination=node_11.

Based on graph partition, we group all nodes in differernt cells(partition), the connection between cells have optimum minimum cuts.  After custmization, for each cell at different level, the cost metrix between inner nodes and outer nodes has been built.

Let's assumn node_11 is the one inside node of cell_1_5 with no connection to outside.  If we got all shortest path from node_0 to cell_1_5, then the shortest path from node_0 to node_11 equals min{shortest path from node_0 to cell_1_5 + shortest path from enter point to node_11}.  

<img src="../references/pictures/mld_routing_example_backward_1.png" alt="mld_routing_example_backward_1" width="100"/>


Let's assume we got all shortest path from node_0 to cell_2_3, then we could easily got shortest path from node 0 to cell_1_5, because this two cell contains the same informtion.    


<img src="../references/pictures/mld_routing_example_backward_2.png" alt="mld_routing_example_backward_2" width="100"/>



Let's assume we got all shortest path from node_0 to cell_3_1, how can we get all shortest path to cell_2_3 quickly?  Actually, we could calculate route at cell_2_* level, treat cell_2_1, cell_2_2, cell_2_3 as a "big node", and put entrance segments at cell_3_* level to the priority_queue and only explore at level_2 level.  By this optimization, we could ignore all inner elements inside of cell_2_1 and cell_2_2 and we won's miss best solution.  The less connection between cells, the faster speed we could get for route calculation.  

<img src="../references/pictures/mld_routing_example_backward_3.png" alt="mld_routing_example_backward_3" width="200"/>

Then, find shortest path from node_0 to node_11 convert to find shortest path from node_0 to cell_3_1.  As long as we calculate all cost from node_0 to cell_3_1, we could find shortest path for the final result.  

<img src="../references/pictures/mld_routing_example_backward_4.png" alt="mld_routing_example_backward_4" width="100"/>

We can achieve that with similar strategy:

<img src="../references/pictures/mld_routing_example_forward_2.png" alt="mld_routing_example_forward_2" width="200"/>

At beginning, we are at level 0 and will calculate all possible shortest path to the boarder of cell_1_0.  Then we could have the view of cell_1_0, everything matters is cell_1_*.  Which means, we don't care what's inside cell_1_1, we treat which as "big node" the only thing matters is its enter edges and exit edges.

<img src="../references/pictures/mld_routing_example_forward_3.png" alt="mld_routing_example_forward_3" width="200"/>

After we have all shortest path to the boarder of cell_2_0, we have the view of cell_2_0, everything matters is level 2 now.  Then we could easily promote to cell_3_0.  


<img src="../references/pictures/mld_routing_example_forward_4.png" alt="mld_routing_example_forward_4" width="200"/>

Then, we just need consider cell_3_0, cell_3_2, cell_3_1 and we could gurantee to calculate all shortest path from node_0 to cell_3_1.  
<img src="../references/pictures/mld_routing_example_level3.png" alt="mld_routing_example_level3" width="200"/>



Summary:
- Based on CRP, MLD could calculate the shortest path very quickly.  The result of graph partition(balanced min-cut) is critical to the performance of MLD.  
- MLD has the ability to level up(from node_0->cell_1_0->cell_2_0->cell_3_0) and level down(cell_3_1->cell_2_3->cell_1_5->node_11)  


