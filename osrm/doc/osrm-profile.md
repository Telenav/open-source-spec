# OSRM Profile

## Files Size & Memory 
See [Toolchain file overview](https://github.com/Project-OSRM/osrm-backend/wiki/Toolchain-file-overview) for description of each file.     

### us-west CH

```bash
$ # original us-west OSM PBF data file
$ ll -lh
$ -rw-rw-r-- 1 mapuser mapuser 1.6G Sep 29 18:36 20180929T181820CST-us-west-latest.osm.pbf
$ 
$ # generated OSRM data files by algorithm=CH, profile=car 
$ ll -lh
total 5.6G
-rw-r--r-- 1 mapuser appuser 136M Sep 30 11:10 20180929T181820CST-us-west-latest.osrm.cnbg
-rw-r--r-- 1 mapuser appuser 136M Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.cnbg_to_ebg
-rw-r--r-- 1 mapuser appuser  68K Sep 30 11:18 20180929T181820CST-us-west-latest.osrm.datasource_names
-rw-r--r-- 1 mapuser appuser 638M Sep 30 11:18 20180929T181820CST-us-west-latest.osrm.ebg
-rw-r--r-- 1 mapuser appuser 200M Sep 30 11:18 20180929T181820CST-us-west-latest.osrm.ebg_nodes
-rw-r--r-- 1 mapuser appuser 224M Sep 30 11:17 20180929T181820CST-us-west-latest.osrm.edges
-rw-r--r-- 1 mapuser appuser 130M Sep 30 11:17 20180929T181820CST-us-west-latest.osrm.enw
-rwx------ 1 mapuser appuser 759M Sep 30 11:18 20180929T181820CST-us-west-latest.osrm.fileIndex
-rw-r--r-- 1 mapuser appuser 856M Sep 30 11:17 20180929T181820CST-us-west-latest.osrm.geometry
-rw-r--r-- 1 mapuser appuser 1.5G Sep 30 13:23 20180929T181820CST-us-west-latest.osrm.hsgr
-rw-r--r-- 1 mapuser appuser 152M Sep 30 11:17 20180929T181820CST-us-west-latest.osrm.icd
-rw-r--r-- 1 mapuser appuser 6.0K Sep 30 11:13 20180929T181820CST-us-west-latest.osrm.maneuver_overrides
-rw-r--r-- 1 mapuser appuser  12M Sep 30 11:06 20180929T181820CST-us-west-latest.osrm.names
-rw-r--r-- 1 mapuser appuser 460M Sep 30 11:10 20180929T181820CST-us-west-latest.osrm.nbg_nodes
-rw-r--r-- 1 mapuser appuser 6.0K Sep 30 11:06 20180929T181820CST-us-west-latest.osrm.properties
-rw-r--r-- 1 mapuser appuser 3.1M Sep 30 11:18 20180929T181820CST-us-west-latest.osrm.ramIndex
-rw-r--r-- 1 mapuser appuser 4.0K Sep 30 11:13 20180929T181820CST-us-west-latest.osrm.restrictions
-rw-r--r-- 1 mapuser appuser  10K Sep 30 11:17 20180929T181820CST-us-west-latest.osrm.tld
-rw-r--r-- 1 mapuser appuser  17K Sep 30 11:17 20180929T181820CST-us-west-latest.osrm.tls
-rw-r--r-- 1 mapuser appuser  64M Sep 30 11:13 20180929T181820CST-us-west-latest.osrm.turn_duration_penalties
-rw-r--r-- 1 mapuser appuser 383M Sep 30 11:13 20180929T181820CST-us-west-latest.osrm.turn_penalties_index
-rw-r--r-- 1 mapuser appuser  64M Sep 30 11:13 20180929T181820CST-us-west-latest.osrm.turn_weight_penalties

```

```bash
$ top
$
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND         
12087 root      20   0 6771824 3.175g   1092 S   0.0 10.2   0:35.01 osrm-routed     

```

### us-west MLD

```bash
$ # original us-west OSM PBF data file
$ ll -lh
$ -rw-rw-r-- 1 mapuser mapuser 1.6G Sep 29 18:36 20180929T181820CST-us-west-latest.osm.pbf
$ 
$ # generated OSRM data files by algorithm=MLD, profile=car 
$ ll -lh
total 6.3G
-rw-r--r-- 1 mapuser appuser 1.3G Sep 30 11:26 20180929T181820CST-us-west-latest.osrm.cell_metrics
-rw-r--r-- 1 mapuser appuser  14M Sep 30 11:22 20180929T181820CST-us-west-latest.osrm.cells
-rw-r--r-- 1 mapuser appuser 136M Sep 30 11:06 20180929T181820CST-us-west-latest.osrm.cnbg
-rw-r--r-- 1 mapuser appuser 136M Sep 30 11:21 20180929T181820CST-us-west-latest.osrm.cnbg_to_ebg
-rw-r--r-- 1 mapuser appuser  68K Sep 30 11:23 20180929T181820CST-us-west-latest.osrm.datasource_names
-rw-r--r-- 1 mapuser appuser 638M Sep 30 11:22 20180929T181820CST-us-west-latest.osrm.ebg
-rw-r--r-- 1 mapuser appuser 200M Sep 30 11:21 20180929T181820CST-us-west-latest.osrm.ebg_nodes
-rw-r--r-- 1 mapuser appuser 224M Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.edges
-rw-r--r-- 1 mapuser appuser 130M Sep 30 11:21 20180929T181820CST-us-west-latest.osrm.enw
-rwx------ 1 mapuser appuser 759M Sep 30 11:21 20180929T181820CST-us-west-latest.osrm.fileIndex
-rw-r--r-- 1 mapuser appuser 856M Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.geometry
-rw-r--r-- 1 mapuser appuser 152M Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.icd
-rw-r--r-- 1 mapuser appuser 6.0K Sep 30 11:21 20180929T181820CST-us-west-latest.osrm.maneuver_overrides
-rw-r--r-- 1 mapuser appuser 729M Sep 30 11:26 20180929T181820CST-us-west-latest.osrm.mldgr
-rw-r--r-- 1 mapuser appuser  12M Sep 30 11:01 20180929T181820CST-us-west-latest.osrm.names
-rw-r--r-- 1 mapuser appuser 460M Sep 30 11:06 20180929T181820CST-us-west-latest.osrm.nbg_nodes
-rw-r--r-- 1 mapuser appuser 130M Sep 30 11:22 20180929T181820CST-us-west-latest.osrm.partition
-rw-r--r-- 1 mapuser appuser 6.0K Sep 30 11:01 20180929T181820CST-us-west-latest.osrm.properties
-rw-r--r-- 1 mapuser appuser 3.1M Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.ramIndex
-rw-r--r-- 1 mapuser appuser 4.0K Sep 30 11:08 20180929T181820CST-us-west-latest.osrm.restrictions
-rw-r--r-- 1 mapuser appuser  10K Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.tld
-rw-r--r-- 1 mapuser appuser  17K Sep 30 11:11 20180929T181820CST-us-west-latest.osrm.tls
-rw-r--r-- 1 mapuser appuser  64M Sep 30 11:08 20180929T181820CST-us-west-latest.osrm.turn_duration_penalties
-rw-r--r-- 1 mapuser appuser 383M Sep 30 11:08 20180929T181820CST-us-west-latest.osrm.turn_penalties_index
-rw-r--r-- 1 mapuser appuser  64M Sep 30 11:08 20180929T181820CST-us-west-latest.osrm.turn_weight_penalties
```

```bash
$ top
$
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND         
10817 root      20   0 7638032 4.213g   2728 S   0.0 13.5   0:38.28 osrm-routed     

```

### california(telenav_osm) MLD 

```bash
$ 
$ # generated OSRM data files by algorithm=MLD, profile=car 
$ ll -lh
total 3.9G
-rw-r--r-- 1 mapuser appuser 633M Nov 13 02:24 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm
-rw-r--r-- 1 mapuser appuser 755M Dec 11 08:20 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.cell_metrics
-rw-r--r-- 1 mapuser appuser 5.1M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.cells
-rw-r--r-- 1 mapuser appuser  49M Nov 13 02:25 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.cnbg
-rw-r--r-- 1 mapuser appuser  49M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.cnbg_to_ebg
-rw-r--r-- 1 mapuser appuser  68K Dec 11 08:19 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.datasource_names
-rw-r--r-- 1 mapuser appuser 273M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.ebg
-rw-r--r-- 1 mapuser appuser  72M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.ebg_nodes
-rw-r--r-- 1 mapuser appuser  80M Nov 13 02:27 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.edges
-rw-r--r-- 1 mapuser appuser  70M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.enw
-rwx------ 1 mapuser appuser 253M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.fileIndex
-rw-r--r-- 1 mapuser appuser 288M Nov 13 02:27 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.geometry
-rw-r--r-- 1 mapuser appuser 679M Dec  8 02:29 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.hsgr
-rw-r--r-- 1 mapuser appuser  50M Nov 13 02:27 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.icd
-rw-r--r-- 1 mapuser appuser 6.0K Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.maneuver_overrides
-rw-r--r-- 1 mapuser appuser 284M Dec 11 08:20 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.mldgr
-rw-r--r-- 1 mapuser appuser 4.3M Nov 13 02:24 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.names
-rw-r--r-- 1 mapuser appuser 151M Nov 13 02:25 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.nbg_nodes
-rw-r--r-- 1 mapuser appuser  47M Dec  8 02:00 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.partition
-rw-r--r-- 1 mapuser appuser 6.0K Nov 13 02:24 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.properties
-rw-r--r-- 1 mapuser appuser 1.1M Nov 13 02:27 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.ramIndex
-rw-r--r-- 1 mapuser appuser 4.0K Nov 13 02:26 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.restrictions
-rw-r--r-- 1 mapuser appuser 8.5K Nov 13 02:27 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.tld
-rw-r--r-- 1 mapuser appuser  14K Nov 13 02:27 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.tls
-rw-r--r-- 1 mapuser appuser  23M Nov 13 02:26 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.turn_duration_penalties
-rw-r--r-- 1 mapuser appuser 137M Nov 13 02:26 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.turn_penalties_index
-rw-r--r-- 1 mapuser appuser  23M Nov 13 02:26 OSM_PBF_US-CA_OSM_18M8D9_Adapter_1.0.0.SNAPSHOT_1533820276000_dev.osrm.turn_weight_penalties
```

```bash
$ top
$
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND         
 2831 mapuser   20   0 4252364 2.419g   3320 S   0.0  7.7   0:09.34 osrm-routed     

```