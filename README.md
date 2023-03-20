VAL
======


Vehicle Adaptation Layer (VAL) is an adaptation of ndnSim that allows for geographical-based forwarding in VANETs.
 
VAL endows ndnSim with a second forwarding plane capable of making forwarding decisions based on the destination area of a packet and the position of the previous-hop.

VAL serves the NFD forwarding plane, does not replace it. It respects layer independence and does not alter the NDN primitives or core structures.

VAL implements a forwarding strategy based on distances, other forwarding strategies can be developed using \c ValStrategy as base class.

The design provides:
 - (1) a mechanism to bind NDN data names to the producers’ and consumers' geographic areas; (geoFaces)
 - (2) an algorithm to guide Interests towards data producers using the distances between nodes and distances to the producers' areas;  (`ValDistancesStrategy`)
 - (3) an algorithm to guide Data packets towards data consumers' using the distances between nodes; (`ValDistancesStrategy`)
 - (4) a discovery mechanism to find producers' areas. (discovery phase: using a special geoFace with no associated geo-area that is the default route in every FIB, the floodFace)
 - (5) a mechanism to mitigate Broadcast Storms. (Forwarding cancellation based on timers and retransmission of packets from node in better position)
 - (6) a mechanism that mitigates intermittent communications characteristic of VANETs (implicit acknowlegment)
 
 This work is based on the LAL presented in [NAVIGO](https://ieeexplore.ieee.org/abstract/document/7158165)
 
 VAL is still in an embryonic stage of development.

 Our scenario can be found in:
 ```shell
    ndnSIM/examples/ndn-grid-v2v-simple.cpp
 ```

 Our mobility tracers in:
 ```shell
    ndnSIM/examples/mobility-tracers/
 ```

 To run the proof of concept scenario (under the ns3 folder):

Create  a dir for the results
```shell
mkdir R
```

Copy the script that treats the raw results
```shell
cp src/ndnSim/scripts/bulk_tester.sh R/
```

Copy the Rscripts
```shell
cp src/ndnSim/scripts/eval4.R R/
cp src/ndnSim/scripts/evalGraph.R R/
cp src/ndnSim/scripts/header_results.txt R/
```

Copy the script that executes the simulations
```shell
cp src/ndnSim/scripts/multirunner.sh .
```

### multirunner.sh
As it is, this script runs 120 sims. 
 - 30 sims with 100 vehicles
 - 30 sims with 70 vehicles
 - 30 sims with 50 vehicles
 - 30 sims with 30 vehicles
For each sim, app-delays-trace and rate-trace files are generated.
Each sim also produces a val_pkts file per node. These files contain information about packets from VAL's perspective. The script combines all the val_pkts files in just one s${i}_val_pkts.txt file, creating a simulation view of VAL's behaviour. 

These files are then moved to the R directory, and any unnecessary files are removed. This process continues until the last sim is done.

The script then invokes the bulk_tester.sh in the R directory.

### bulk_tester.sh
This bash script invokes the Rscript (eval4.R) that treats the raw metrics output of ndnSIM and generates a result file per sim. This bash script combines all the results of individual sims into one file final_results.csv
This bash script also invokes the Rscript responsible for generating the charts (evalGraph.R).

## run
```shell
sh multirunner.sh
```

VAL documentation can be found [here](https://jfpereira88.github.io/VAL/)
Our results for this scenario and a complete description of our metrics can be found [here](https://marco.uminho.pt/data-archive/papers/)


----



ndnSIM
======

[![Build Status](https://travis-ci.org/named-data-ndnSIM/ndnSIM.svg)](https://travis-ci.org/named-data-ndnSIM/ndnSIM)

A new release of [NS-3 based Named Data Networking (NDN) simulator](http://ndnsim.net/)
went through extensive refactoring and rewriting.  The key new features of the new
version:

- [NDN Packet Specification](http://named-data.net/doc/NDN-packet-spec/current/)

- ndnSIM uses implementation of basic NDN primitives from
  [ndn-cxx library (NDN C++ library with eXperimental eXtensions)](http://named-data.net/doc/ndn-cxx/)

  Based on version `0.6.5`

- All NDN forwarding and management is implemented directly using source code of
  [Named Data Networking Forwarding Daemon (NFD)](http://named-data.net/doc/NFD/)

  Based on version `0.6.5`

- Allows [simulation of real applications](http://ndnsim.net/guide-to-simulate-real-apps.html)
  written against ndn-cxx library

- Requires a modified version of NS-3 based on version `ns-3.29`

[ndnSIM documentation](http://ndnsim.net)
---------------------------------------------

For more information, including downloading and compilation instruction, please refer to
http://ndnsim.net or documentation in `docs/` folder.
