#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/socket.h"
#include "ns3/packet.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Simulation");

class RespondingApp : public Application {
public:
    RespondingApp() : m_packetSize(1024), m_totalRx(0), m_threshold(4096) {}
    virtual ~RespondingApp() {}

    void AddLocalPort(uint16_t localPort) {
        m_localPorts.push_back(localPort);
    }

    void AddPeerAddress(Address peerAddress) {
        m_peerAddresses.push_back(peerAddress);
    }

protected:
    virtual void StartApplication() override {
        for (uint16_t port : m_localPorts) {
            Ptr<Socket> socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), port));
            socket->SetRecvCallback(MakeCallback(&RespondingApp::HandleRead, this));
            m_sockets.push_back(socket);
        }
        std::cout << "Application started, listening on multiple ports." << std::endl;
    }

    virtual void StopApplication() override {
        for (Ptr<Socket> socket : m_sockets) {
            if (socket) {
                socket->Close();
            }
        }
        m_sockets.clear();
        std::cout << "Application stopped." << std::endl;
    }

    void HandleRead(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        while ((packet = socket->Recv())) {
            uint32_t packetSize = packet->GetSize();
            m_totalRx += packetSize;
            std::cout << "Received packet of size " << packetSize << " bytes, total received: " << m_totalRx << " bytes" << std::endl;
            if (m_totalRx >= m_threshold) {
                SendFeedback();
                m_totalRx = 0;  // Reset the counter after sending feedback
            }
        }
    }

    void SendFeedback() {
    Ptr<Packet> packet = Create<Packet>(m_packetSize);  // Creating a packet of the specified size
    Ptr<Socket> socket = m_sockets[0];

    // Assume that each socket in m_sockets corresponds to each address in m_peerAddresses at the same index
    for (size_t i = 0; i < m_peerAddresses.size(); i++) {
        Address peerAddress = m_peerAddresses[i];
        if (socket->SendTo(packet, 0, peerAddress) < 0) {
            std::cerr << "SendTo failed to " << InetSocketAddress::ConvertFrom(peerAddress).GetIpv4() << std::endl;
        } else {
            std::cout << "Sent feedback packet of size " << m_packetSize << " bytes to "
                      << InetSocketAddress::ConvertFrom(peerAddress).GetIpv4() << std::endl;
        }
    }
}

private:
    std::vector<Ptr<Socket>> m_sockets;  // 网络套接字列表
    std::vector<uint16_t> m_localPorts;  // 本地端口列表
    std::vector<Address> m_peerAddresses;  // 远端地址列表
    uint32_t m_packetSize;  // 数据包大小
    uint32_t m_totalRx;  // 总接收字节
    uint32_t m_threshold;  // 触发反馈的阈值
};

int main(int argc, char *argv[]) {
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    NodeContainer nodes;
    nodes.Create(5);  // Now creating five nodes

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Create point-to-point links
    NetDeviceContainer devices02 = pointToPoint.Install(nodes.Get(0), nodes.Get(2));
    NetDeviceContainer devices12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer devices23 = pointToPoint.Install(nodes.Get(2), nodes.Get(3));
    NetDeviceContainer devices24 = pointToPoint.Install(nodes.Get(2), nodes.Get(4));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces02 = address.Assign(devices02);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces23 = address.Assign(devices23);
    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces24 = address.Assign(devices24);

    uint16_t receiver_port = 9;
    UdpClientHelper client0(interfaces02.GetAddress(1), receiver_port);
    client0.SetAttribute("MaxPackets", UintegerValue(30));
    client0.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client0.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps0 = client0.Install(nodes.Get(0));
    clientApps0.Start(Seconds(1.0));
    clientApps0.Stop(Seconds(30.0));

    UdpClientHelper client1(interfaces12.GetAddress(1), receiver_port);
    client1.SetAttribute("MaxPackets", UintegerValue(30));
    client1.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client1.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps1 = client1.Install(nodes.Get(1));
    clientApps1.Start(Seconds(1.0));
    clientApps1.Stop(Seconds(30.0));

    Ptr<RespondingApp> app = CreateObject<RespondingApp>();
    app->AddLocalPort(receiver_port);
    app->AddPeerAddress(InetSocketAddress(interfaces23.GetAddress(1), receiver_port)); //InetSocketAddress(interfaces12.GetAddress(1), port)
    app->AddPeerAddress(InetSocketAddress(interfaces24.GetAddress(1), receiver_port));
    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(30.0));
    nodes.Get(2)->AddApplication(app);

    AnimationInterface anim("./scratch/detector.xml");
    anim.SetConstantPosition(nodes.Get(0), 20, 20);
    anim.SetConstantPosition(nodes.Get(1), 20, 80);
    anim.SetConstantPosition(nodes.Get(2), 50, 50);
    anim.SetConstantPosition(nodes.Get(3), 80, 20);
    anim.SetConstantPosition(nodes.Get(4), 80, 80);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
