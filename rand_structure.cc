#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    // Create 20 nodes
    NodeContainer nodes;
    nodes.Create(20);

    // Setup point-to-point helper with default attributes
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Install network stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Randomly place nodes within a 100x100 grid
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Random number generator for node connections
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    uv->SetAttribute("Min", DoubleValue(0));
    uv->SetAttribute("Max", DoubleValue(19));

    // Create random links
    NetDeviceContainer devices;
    int numberOfLinks = 30;
    for (int i = 0; i < numberOfLinks; i++) {
        int nodeA = uv->GetInteger();
        int nodeB = uv->GetInteger();
        if (nodeA != nodeB) {
            NetDeviceContainer linkDevices = pointToPoint.Install(nodes.Get(nodeA), nodes.Get(nodeB));
            devices.Add(linkDevices);
        } else {
            i--; // Decrement to retry this iteration
        }
    }

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Setup animation
    AnimationInterface anim("network-animation.xml");  // File for animation output
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<Node> node = nodes.Get(i);
        Vector position = node->GetObject<MobilityModel>()->GetPosition();
        anim.SetConstantPosition(node, position.x, position.y);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
