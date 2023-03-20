#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "ns3/ndnSIM-module.h"
#include "ns3/ndnSIM/model/ndn-block-header.hpp"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>

#include <algorithm>
#include <vector>

namespace ns3{


NS_LOG_COMPONENT_DEFINE ("v2v-grid");


void installMobility(std::string traceFile)
{
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install (); // configure movements for each node, while reading trace file

}

void install80211p(NodeContainer &c, NetDeviceContainer &devices)
{
    bool verbose (false);
    std::string phyMode ("OfdmRate6MbpsBW10MHz");
    //std::string phyMode("OfdmRate24Mbps");

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper ();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();

    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",
                                 "MaxRange", DoubleValue(110.0));
    wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel",
                                 "m0", DoubleValue(1.0),
                                 "m1", DoubleValue(1.0),
                                 "m2", DoubleValue(1.0));


    Ptr<YansWifiChannel> channel = wifiChannel.Create ();
    wifiPhy.SetChannel (channel);
    // ns-3 supports generate a pcap trace
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
    NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
    Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
    if (verbose) {
        wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }

    wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode));
    devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);

    // Tracing
    //wifiPhy.EnablePcap ("ndn-v2v-grid", devices);
}

void installNDN(NodeContainer &c)
{
  ndn::StackHelper ndnHelper;
  
  ndnHelper.SetDefaultRoutes (false);
  ndnHelper.Install (c);
  ndn::StrategyChoiceHelper::InstallAll ("/", "/localhost/nfd/strategy/broadcast");
}

void installConsumer (NodeContainer &c)
{
  ndn::AppHelper helper ("ns3::ndn::ConsumerCbr");

  helper.SetAttribute ("Frequency", DoubleValue (1.0));
  helper.SetAttribute ("Randomize", StringValue("none"));
  helper.SetPrefix ("/v2v/test");
  helper.Install (c);
}

void installProducer (NodeContainer &c)
{
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");

  producerHelper.SetPrefix ("/v2v/grid-test");
  producerHelper.Install (c);
}

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("NDN-GRID-V2V");

  std::string traceFile = "/home/jfpereira/opt/ndnSIM-VAL/ns-3/src/ndnSIM/examples/mobility-tracers/v2v-grid-1.tcl";

  const int producerNodes[2] = {0, 15};

  const int consumerNodes[10] = {1, 2, 3, 4, 5, 10, 11, 12, 13, 14};

  int    nodeNum = 30;
  double duration = 600;
  

  NodeContainer c;
  c.Create (nodeNum);

  installMobility (traceFile);

  NetDeviceContainer netDevices;
  install80211p (c, netDevices);
  
  installNDN (c);

  NodeContainer producer;
  for (int n : producerNodes)
    producer.Add (c.Get(n));

  NodeContainer consumers;
  for (int n : consumerNodes)
    consumers.Add (c.Get(n));

  installConsumer (consumers);
  installProducer (producer);


  Simulator::Stop (Seconds (duration));


  //ndn::AppDelayTracer::InstallAll("app-delays-trace.txt");
  //ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(0.5));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}