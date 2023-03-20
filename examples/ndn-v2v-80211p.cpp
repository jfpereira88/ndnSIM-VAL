#include "ns3/core-module.h"

#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"
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

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

namespace ns3{


NS_LOG_COMPONENT_DEFINE ("V2V-802.11p");


static void
CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity

  // Prints position and velocities
  *os << Simulator::Now () << " POS: x=" << pos.x << ", y=" << pos.y
      << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
      << ", z=" << vel.z << std::endl;
}


void installMobility(NodeContainer &c, std::string traceFile)
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

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                 "MaxRange", DoubleValue(110.0));
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
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
    wifiPhy.EnablePcap ("wave-simple-80211p", devices);
}

void installNDN(NodeContainer &c)
{
  ndn::StackHelper ndnHelper;
  
  ndnHelper.SetDefaultRoutes(false);
  
  ndnHelper.Install(c);
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/broadcast");
}

void installConsumer(NodeContainer &c)
{
  ndn::AppHelper helper("ns3::ndn::ConsumerCbr");
  helper.SetAttribute("Frequency", DoubleValue (1.0));
  helper.SetAttribute("Randomize", StringValue("none"));
  helper.SetPrefix("/v2v/test");

  helper.Install(c);
  NS_LOG_INFO("Consumer installed on node " << c.Get(0)->GetId());
}

void installProducer(NodeContainer &c)
{
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/v2v/test");

  producerHelper.Install(c);
  NS_LOG_INFO("Producer installed on node " << c.Get(0)->GetId());
}

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("V2VTest Simulator");
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  std::string traceFile;
  std::string logFile;

  const int producerNodes[15] =            { 0,  4,  19,  21,  35,  37,  45,  50,  52,  55,  58,  59,  66,  67,  68};
  const int produderTerminationTimes[15] = {73, 67, 125, 152, 177, 181, 204, 224, 206, 210, 254, 248, 248, 248, 248}; 

  const int consumerNodes[25] =            {2,   5,  16,  17,  18,  20,  31,  34,  36,  40,  46,  47,  49,  53,  54,  56,  57,  60,  61,  62,  63,  64,  65,  69,  70};
  const int consumerTerminationTimes[25] = {62, 75, 120, 135, 149, 103, 145, 189, 163, 184, 208, 248, 221, 216, 216, 236, 215, 230, 247, 236, 248, 248, 248, 248, 248};

  int    nodeNum;
  double duration;
  

  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue ("duration", "Duration of Simulation", duration);
  cmd.AddValue ("logFile", "Log file", logFile);
  cmd.Parse (argc,argv);

  if (traceFile.empty () || nodeNum <= 0 || duration <= 0 || logFile.empty ())
  {
    return 0;
  }


  NodeContainer c;
  c.Create(nodeNum);

  installMobility(c, traceFile);

  NetDeviceContainer netDevices;
  install80211p(c, netDevices);
  
  installNDN(c);
  
  //setting application
  //Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  uint32_t producerId = 5;//randomNum->GetValue(0,numNodes-1);
  uint32_t consumerId1 = 2;

  NodeContainer producer;
  for (int n : producerNodes)
    producer.Add(c.Get(n));

  NodeContainer consumers;
  for (int n : consumerNodes)
    consumers.Add(c.Get(n));

  installConsumer(consumers);
  installProducer(producer);


  Simulator::Stop(Seconds(duration));

  std::ofstream os;
  os.open (logFile.c_str ());

  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                   MakeBoundCallback (&CourseChange, &os));

  ndn::AppDelayTracer::InstallAll("app-delays-trace.txt");
  ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(0.5));

  Simulator::Run ();
  Simulator::Destroy ();

  os.close (); // close log file
  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}