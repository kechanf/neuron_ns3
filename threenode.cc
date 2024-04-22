#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SingleWayUdpCommunication");

int main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices12, devices13;
    devices12 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    devices13 = pointToPoint.Install(nodes.Get(0), nodes.Get(2));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces13 = address.Assign(devices13);

    // Set up UdpServer on nodes 1 and 2
    uint16_t serverPort = 4000;
    UdpServerHelper server1(serverPort), server2(serverPort);
    ApplicationContainer serverApps1 = server1.Install(nodes.Get(1));
    serverApps1.Start(Seconds(1.0));
    serverApps1.Stop(Seconds(10.0));
    ApplicationContainer serverApps2 = server2.Install(nodes.Get(2));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(10.0));

    // Set up UdpClient on node 0 to send to node 1 and node 2
    uint32_t MaxPacketSize = 1024;
    Time interPacketInterval = Seconds(1.0);
    uint32_t maxPacketCount = 5;
    UdpClientHelper client1(interfaces12.GetAddress(1), serverPort);
    client1.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client1.SetAttribute("Interval", TimeValue(interPacketInterval));
    client1.SetAttribute("PacketSize", UintegerValue(MaxPacketSize));
    ApplicationContainer clientApps1 = client1.Install(nodes.Get(0));
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(10.0));

    UdpClientHelper client2(interfaces13.GetAddress(1), serverPort);
    client2.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client2.SetAttribute("Interval", TimeValue(interPacketInterval));
    client2.SetAttribute("PacketSize", UintegerValue(MaxPacketSize));
    ApplicationContainer clientApps2 = client2.Install(nodes.Get(0));
    clientApps2.Start(Seconds(2.0));
    clientApps2.Stop(Seconds(10.0));

    AnimationInterface anim("animation.xml");
    anim.SetConstantPosition(nodes.Get(0), 1.0, 2.0);
    anim.SetConstantPosition(nodes.Get(1), 20.0, 30.0);
    anim.SetConstantPosition(nodes.Get(2), 30.0, 20.0);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
