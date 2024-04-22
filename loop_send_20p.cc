#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SequentialUdpCommunication");

int main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(21);  // 1 sender, 20 receivers

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    NetDeviceContainer devices;
    std::vector<Ipv4InterfaceContainer> interfaces;

    // Install and assign IP to point-to-point devices
    for (int i = 1; i < 21; ++i) {
        devices = pointToPoint.Install(nodes.Get(0), nodes.Get(i));
        std::ostringstream subnet;
        subnet << "10.1." << i << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        interfaces.push_back(address.Assign(devices));
    }

    // Setup UdpServer on all receiver nodes
    uint16_t serverPort = 4000;
    UdpServerHelper server(serverPort);

    ApplicationContainer serverApps;
    for (int i = 1; i < 21; ++i) {
        serverApps.Add(server.Install(nodes.Get(i)));
    }

    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));

    // Setup UdpClient on the sender node to send to each receiver node sequentially
    for (int i = 0; i < 20; ++i) {
        UdpClientHelper client(interfaces[i].GetAddress(1), serverPort);
        client.SetAttribute("MaxPackets", UintegerValue(1));
        client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        client.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApps = client.Install(nodes.Get(0));
        clientApps.Start(Seconds(2.0 + i));
        clientApps.Stop(Seconds(3.0 + i));  // Ensure each client only sends once
    }

    AnimationInterface anim("sequential-udp-animation.xml");
    anim.SetConstantPosition(nodes.Get(0), 1.0, 2.0);
    for (int i = 1; i < 21; ++i) {
        anim.SetConstantPosition(nodes.Get(i), 2.0 + i, 2.0);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
