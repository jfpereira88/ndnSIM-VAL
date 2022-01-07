#include "ns3/core-module.h"

#include "ns3/mobility-module.h"
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
#include "ns3/animation-interface.h"

#include <algorithm>
#include <vector>

namespace ns3{

/**
 * This scenario simulates a scenario with 6 cars movind and communicating
 * in an ad-hoc way.
 *
 * 5 consumers request data from producer with frequency 1 interest per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-v2v-simple
 *
 * To modify the mobility model, see function installMobility.
 * To modify the wifi model, see function installWifi.
 * To modify the NDN settings, see function installNDN and for consumer and
 * producer settings, see functions installConsumer and installProducer
 * respectively.
 */

NS_LOG_COMPONENT_DEFINE ("V2VSimple");

class PcapWriter {
public:
  PcapWriter(const std::string& file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile(file, std::ios::out, PcapHelper::DLT_IEEE802_11_RADIO);
  }

  void
  TracePacket(Ptr<const Packet> packet)
  {
    static WifiMacHeader wifiHeader;

    m_pcap->Write(Simulator::Now(), wifiHeader, packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};


static const uint32_t numNodes = 6;

void printPosition(Ptr<const MobilityModel> mobility) //DEBUG purpose
{
  Simulator::Schedule(Seconds(1), &printPosition, mobility);
  NS_LOG_INFO("Car "<<  mobility->GetObject<Node>()->GetId() << " is at: " <<mobility->GetPosition());
}


void installMobility(NodeContainer &c, int simulationEnd)
{
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::WaypointMobilityModel");
  mobility.Install(c);

  bool test = true;
  if(test){
    Ptr<WaypointMobilityModel> wayMobility[numNodes];
    for (uint32_t i = 0; i < numNodes; i++) {
      wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
      Waypoint waypointStart;
      Waypoint waypointEnd;
      // i*50+138
      waypointStart = Waypoint(Seconds(0), Vector3D(i*50, 0, 0));
      waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(i*50+138, 0, 0));
      /*if(i == 0) {
        waypointStart = Waypoint(Seconds(0), Vector3D(95, 150, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(95, 150, 0));
      }
      if(i == 1) {
        waypointStart = Waypoint(Seconds(0), Vector3D(195, 150, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(195, 150, 0));
      }
      if(i == 2) {
        waypointStart = Waypoint(Seconds(0), Vector3D(295, 150, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(300, 150, 0));
      }
      if(i == 3) {
        waypointStart = Waypoint(Seconds(0), Vector3D(395, 150, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(405, 150, 0));
      }
      if(i == 4) {
        waypointStart = Waypoint(Seconds(0), Vector3D(495, 150, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(505, 150, 0));
      }
      if(i == 5) {
        waypointStart = Waypoint(Seconds(0), Vector3D(595, 150, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(300, 255, 0));
      }
      if(i == 6) {
        waypointStart = Waypoint(Seconds(0), Vector3D(300, 250, 0));
        waypointEnd = Waypoint(Seconds(simulationEnd), Vector3D(300, 255, 0));
      }*/


      wayMobility[i]->AddWaypoint(waypointStart);
      wayMobility[i]->AddWaypoint(waypointEnd);
      NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointEnd);
    }


    return;
  }
  Ptr<WaypointMobilityModel> wayMobility[numNodes];
  for (uint32_t i = 0; i < numNodes; i++) {
    wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
    Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
    Waypoint waypointMiddle(Seconds(simulationEnd/2), Vector3D(i*20+1000, 0, 0));
    Waypoint waypointEnd(Seconds(simulationEnd+1), Vector3D(i*20+1000, 0, 0));

    wayMobility[i]->AddWaypoint(waypointStart);
    wayMobility[i]->AddWaypoint(waypointMiddle);
    wayMobility[i]->AddWaypoint(waypointEnd);
    NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointMiddle << " " << waypointEnd);
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
                                 "MaxRange", DoubleValue(110.0));
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
  wifiPhy.EnablePcap("ndn-simple-v2v", devices);
}

void installNDN(NodeContainer &c)
{
  ndn::StackHelper ndnHelper;
  
  ndnHelper.SetDefaultRoutes(false);
  
  ndnHelper.Install(c);
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/broadcast");
  

  ///todo add v2v face


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
  producerHelper.SetPrefix("/v2v");

  producerHelper.Install(c.Get(0));
  NS_LOG_INFO("Producer installed on node " << c.Get(0)->GetId());

}

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("V2VTest Simulator");
 

  //uint32_t numProducer = 1;
  int simulationEnd = 10;

  NodeContainer c;
  c.Create(numNodes);

  installMobility(c, simulationEnd);

  NetDeviceContainer netDevices;
  installWifi(c, netDevices);
  
  installNDN(c);
  
  //setting application
  //Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  uint32_t producerId = 5;//randomNum->GetValue(0,numNodes-1);
  uint32_t consumerId1 = 2;
  //uint32_t consumerId2 = 5;

  NodeContainer producer;
  producer.Add(c.Get(producerId));

  NodeContainer consumers;
  consumers.Add(c.Get(consumerId1));
  //consumers.Add(c.Get(consumerId2));
  /*
  for(uint32_t i=0; i<numNodes; i++){
    if(i!=producerId){
      consumers.Add(c.Get(i));
      break;//tmp
    }
  }*/

  installConsumer(consumers);
  installProducer(producer);



  /*for(uint32_t i=0; i<c.GetN(); i++){
    Simulator::Schedule(Seconds(1), &printPosition, c.Get(i)->GetObject<WaypointMobilityModel>());
  }*/

  Simulator::Stop(Seconds(simulationEnd));

  //std::string animFile = "v2v-test.xml";
  //AnimationInterface anim(animFile);

  ndn::AppDelayTracer::InstallAll("app-delays-trace.txt");
  ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(0.5));
  //L2RateTracer::InstallAll("drop-trace.txt", Seconds(0.5)); not usefull

  //PcapWriter trace("ndn-simple-v2v-trace.pcap");
  //Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/MacTx",
  //                              MakeCallback(&PcapWriter::TracePacket, &trace));

  Simulator::Run ();
  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}