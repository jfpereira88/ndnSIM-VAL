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

 To run the proof of concept scenario (under the ns3 folder):

```shell
 cp src/ndnSim/examples/ndn-simple-v2v.cpp scratch/ndn-simple-v2v.cc
```
```shell
 NS_LOG="ndn.val.ValForwarder:ndn.val.ValDistancesStrategy:ndn.val.face.ValLinkService:V2VSimple:ndn-cxx.nfd.Forwarder:ndn-cxx.nfd.BroadcastStrategy:ndn.Consumer:ndn.Producer" ./waf --run ndn-simple-v2v
```

VAL documentation can be found [here](https://jfpereira88.github.io/VAL/)


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
