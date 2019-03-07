- [PUNCH](#punch)
  - [Filtering phase](#filtering-phase)
  - [Assembly phase](#assembly-phase)

# PUNCH
PUNCH means partitioning using natural cut heuristics, its intuition is, inside road networks, dense regions(grids, cities) interleaved with natural cuts(mountains, parks, rivers, deserts, sparse areas, freeways).
PUNCH discovered that road networks have remarkably small separators and it appeared to be the one capable of efficiently computing these separators.
PUNCH is divided as filtering phase and Assembly phase

## Filtering phase
Filtering phase will contract dense regions to reduce graph size, while natural cuts structure is preserved. 

<img src="../resource/pictures/natural_cut_point.png" alt="natural_cut_point" width="200"/>

Natural cuts, like the red points in the upper picture, are a group of sparse sets that separate dense areas.

Here is the diagram for how to find them

<img src="../resource/pictures/natural_cut_algorithm.png" alt="natural_cut_algorithm" width="400"/>

Here is the pseudo code

<img src="../resource/pictures/natural_cut_pseudo_code.png" alt="natural_cut_pseudo_code" width="400"/>

Picuture from [Distributed Evolutionary Graph Partitioning](https://arxiv.org/pdf/1110.0477.pdf)

The result of filtering phase is the fragment like below

<img src="../resource/pictures/punch_filter_phase_final.png" alt="punch_filter_phase_final" width="400"/>

## Assembly phase
There are three ingredients, greedy algorithm, local search, multistart and combination heuristics

Greedy algorithm will randomly pick well-connected small fragments and then combine them.  This step will repeat until maximal and finds initial partition.

<img src="../resource/pictures/punch_assembly_greedy_alg.png" alt="punch_assembly_greedy_alg" width="200"/>


The local search will pick two neighboring cells, disassemble them and then apply greedy algorithm to the subproblem.  The logic will be repeatly several times for every pair of neighboring cells.


<img src="../resource/pictures/punch_assembly_phase_local_search.png" alt="punch_assembly_phase_local_search" width="400"/>
