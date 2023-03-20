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

#include<cstdlib>
#include<ctime>

namespace ns3{


NS_LOG_COMPONENT_DEFINE ("v2v-grid");


void installMobility(std::string traceFile)
{
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install (); // configure movements for each node, while reading trace file

}


void instalDarftlMobility(NodeContainer &c, int simulationEnd, int numNodes)
{
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::WaypointMobilityModel");
  mobility.Install(c);

  Ptr<WaypointMobilityModel> wayMobility[numNodes];
  for (int i = 0; i < numNodes; i++)
  {
    wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
    Waypoint waypointStart;
    Waypoint waypointEnd;

    waypointStart = Waypoint(Seconds(0), Vector3D(i*50, 0, 0));
    waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(i*50+840, 0, 0));

    wayMobility[i]->AddWaypoint(waypointStart);
    wayMobility[i]->AddWaypoint(waypointEnd);
  }

}


void installWifi(NodeContainer &c, NetDeviceContainer &devices)
{
  // Modulation and wifi channel bit rate
  std::string phyMode("OfdmRate24Mbps");

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                 "MaxRange", DoubleValue(80.0));
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                 "m0", DoubleValue(1.0),
                                 "m1", DoubleValue(1.0),
                                 "m2", DoubleValue(1.0));
  wifiPhy.SetChannel(wifiChannel.Create());
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  // Add a non-QoS upper mac
  WifiMacHelper wifiMac;

  // Disable rate control
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));

  devices = wifi.Install(wifiPhy, wifiMac, c);
  //wifiPhy.EnablePcap("ndn-simple-v2v", devices);
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
                                 "MaxRange", DoubleValue(80.0));
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

  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(1000);
  ndnHelper.SetDefaultRoutes (false);
  ndnHelper.Install (c);
  ndn::StrategyChoiceHelper::InstallAll ("/", "/localhost/nfd/strategy/broadcast");
}

void installConsumer (NodeContainer &c)
{
  ndn::AppHelper helper ("ns3::ndn::ConsumerCbr");

  helper.SetAttribute ("Frequency", DoubleValue(1));
  helper.SetAttribute ("Randomize", StringValue("uniform"));
  helper.SetAttribute ("LifeTime", TimeValue (Seconds (2)));
  helper.SetAttribute ("RetxTimer", TimeValue (MilliSeconds (500)));
  //helper.SetAttribute ("MaxSeq", IntegerValue (10));
  helper.SetPrefix ("/v2v/test");
  helper.Install (c);
}

void installProducer (NodeContainer &c)
{
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");

  producerHelper.SetPrefix ("/v2v/test");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install (c);
}

void setSeqNum (NodeContainer &consumers)
{
  int seqNoStart = 0;
  int seqNoStep = 120;
  int seqNo = seqNoStart;
  std::ostringstream oss;

  for (auto c : consumers)
  {
    oss.str("");
    oss.clear();
    oss << "/NodeList/" << c->GetId () <<
            "/ApplicationList/0/StartSeq";
    Config::Set(oss.str (), IntegerValue (seqNo));
    seqNo += seqNoStep;
  }

}

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("NDN-GRID-V2V");

  //std::string traceFile = "/home/jfpereira/opt/ndnSIM-VAL/ns-3/src/ndnSIM/examples/mobility-tracers/oneway-2_70_200s.tcl";

  //const int producerNodes[2] = {0, 20};
  //const int producerNodes[1] = {2};
  //const int consumerNodes[10] = {1, 2, 3, 4, 5, 10, 11, 12, 13, 14};
  //const int consumerNodes[4] = {0, 1, 3, 4};

  std::string traceFile;
  int nodeNum;
  int nTries;

  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue ("nTries", "ntries", nTries);
  cmd.Parse (argc,argv);

  if (traceFile.empty () || nodeNum <= 0 || nTries <= 0)
  {
    return 0;
  }

  double duration = 200;
  int consumerNum = 10;
  int producerNum = 2;
  int counter = 0;
  int consumerNodes[consumerNum] = {};
  int producerNodes[producerNum] = {};

  srand(time(0));

  while(counter < consumerNum)
  {
      bool alredySet = false;
      int num = rand()%nodeNum;
      for(int i = 0; i<=counter; i++)
      {
          if(consumerNodes[i] == num)
          {
              alredySet = true;
              break;
          }
      }
      if(!alredySet)
      {
          consumerNodes[counter] = num;
          counter++;
      }
  }

  counter = 0;
  while(counter < producerNum)
  {
      bool alredySet = false;
      int num = rand()%nodeNum;
      for(int i = 0; i<consumerNum; i++)
      {
          if(consumerNodes[i] == num)
          {
              alredySet = true;
              break;
          }
      }
      if(!alredySet)
      {
          for(int i = 0; i<=counter; i++)
          {
              if(producerNodes[i] == num)
              {
                  alredySet = true;
                  break;
              }
          }
      }
      if(!alredySet)
      {
          producerNodes[counter] = num;
          counter++;
      }
  }



  NodeContainer c;
  c.Create (nodeNum);

  installMobility (traceFile);
  //instalDarftlMobility (c, duration, nodeNum);

  NetDeviceContainer netDevices;
  installWifi (c, netDevices);

  installNDN (c);

  NodeContainer producer;
  for (int n : producerNodes)
    producer.Add (c.Get(n));

  NodeContainer consumers;
  for (int n : consumerNodes)
    consumers.Add (c.Get(n));

  installConsumer (consumers);
  installProducer (producer);

  setSeqNum (consumers);


  Simulator::Stop (Seconds (duration));

  std::string app_trace = "app-delays-trace_200s_V"+std::to_string(nodeNum)+"_"+std::to_string(nTries)+".txt";
  std::string rate_trace = "rate-trace_200s_V"+std::to_string(nodeNum)+"_"+std::to_string(nTries)+".txt";

  ndn::AppDelayTracer::InstallAll(app_trace);
  ndn::L3RateTracer::InstallAll(rate_trace, Seconds(0.5));

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
