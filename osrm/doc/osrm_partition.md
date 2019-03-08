# OSRM Partition

OSRM partition generates data for MLD(multiple-level-dijkstra), and its an implementation of [CRP](../../routing_basic/doc/crp.md).<br/>
[Partition](../../routing_basic/doc/graph_partition.md) is the most important step in CRP which would influence custmization speed and query performance dramatically.  OSRM's Partition implement [inertial flow](../../routing_basic/doc/inertial_flow.md), which tries to bi-set graph based on dinic.<br/>
<br/>
Here is a [simple graph](./partition/osrm_partition_simple_case.md) to walk you though OSRM partition's logic, for more details please go to [detail page](./partition/osrm_partition_detail.md).


