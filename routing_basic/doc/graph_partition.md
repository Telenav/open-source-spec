# Graph partition

## General

The graph partitioning problem asks for a division of a graph's node set into k equally sized blocks such that the number of edges that run between the blocks is minimized.  The size of each block is a configurable parameter.

<img src="../resource/pictures/probelm_description_partitioning.png" alt="problem_description_partitioning" style="width:400px;"/>

## Application of the algorithm

| Road network | Graphics process | Scocial Network |
| --- | --- | --- |
| <img src="../resource/pictures/roadnetwork_partition_example_1.png" alt="roadnetwork_partition_example_1" style="width:400px;"/> |  <img src="../resource/pictures/graphics_partition.png" alt="graphics_partition" style="width:400px;"/> |  <img src="../resource/pictures/football_team_match.png" alt="football_team_match" style="width:400px;"/> | 


Graph partition is the first step to distribute processing graph data.

## Ideal partition result

<img src="https://user-images.githubusercontent.com/16873751/104144707-39c08a80-5379-11eb-9e71-fbb180fadf25.png" alt="ideal_partition" width=300/>    <img src="https://user-images.githubusercontent.com/16873751/104144798-9d4ab800-5379-11eb-9471-1f28742aeea3.png" alt="ideal_partition" width=300/>

## Basic Algorithm
- [Kernighan Lin Algorithm](./kernighan_lin_alg.md)

- [Max Flow Min Cut](./max_flow_min_cut.md)

## Solutions for Route network

- [METIS](./metis.md)

- [PUNCH](./punch.md)

- [Inertial Flow](./inertial_flow.md)

- [KAHIP](https://github.com/schulzchristian/KaHIP)

## Distribute Partition
- [pregel](https://kowshik.github.io/JPregel/pregel_paper.pdf)

- [Google - Affinity Clustering: Hierarchical Clustering at Scale](https://papers.nips.cc/paper/7262-affinity-clustering-hierarchical-clustering-at-scale.pdf)

- [Facebook - Balanced Label Propagation for Partitioning Massive Graphs](https://stanford.edu/~jugander/papers/wsdm13-blp.pdf)

## More information
- [Graph Partitioning and Clustering in Theory and Practice by Christian Schulz from KIT](https://par.tuwien.ac.at/teaching/2015w/195.084.psp#detailedcontent)

