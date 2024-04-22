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
    RespondingApp() : m_socket(0), m_packetSize(1024), m_localPort(0),  m_totalRx(0), m_threshold(2048) {}
    virtual ~RespondingApp() {}

    void Setup(uint16_t localPort, Address peerAddress) {
        m_localPort = localPort;
        m_peerAddress = peerAddress;
    }

protected:
    virtual void StartApplication() override {
        if (!m_socket) {
            m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_localPort));  // 绑定本地端口
        }
        m_socket->SetRecvCallback(MakeCallback(&RespondingApp::HandleRead, this));
        std::cout << "Application started, listening on port: " << m_localPort << std::endl;
    }

    virtual void StopApplication() override {
        if (m_socket) {
            m_socket->Close();
            m_socket = 0;
            std::cout << "Application stopped." << std::endl;
        }
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
        Ptr<Packet> packet = Create<Packet>(m_packetSize);  // 创建一个指定大小的数据包
        if (m_socket->SendTo(packet, 0, m_peerAddress) < 0) {
            std::cerr << "SendTo failed to " << InetSocketAddress::ConvertFrom(m_peerAddress).GetIpv4()
                       << std::endl;
        } else {
            std::cout << "Sent feedback packet of size " << m_packetSize << " bytes to "
                      << InetSocketAddress::ConvertFrom(m_peerAddress).GetIpv4() << std::endl;
        }
    }

private:
    Ptr<Socket> m_socket;
    Address m_peerAddress;
    uint32_t m_packetSize;
    uint16_t m_localPort;

    uint32_t m_totalRx;  // Total bytes received
    uint32_t m_threshold;  // Threshold in bytes to send feedback
};

int main(int argc, char *argv[]) {
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
//    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devices12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);

    uint16_t port = 9;
    uint16_t serverPort = 1000;

    UdpClientHelper client(interfaces01.GetAddress(1), serverPort);
    client.SetAttribute("MaxPackets", UintegerValue(30));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = client.Install(nodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(30.0));

    Ptr<RespondingApp> app = CreateObject<RespondingApp>();
    std::cout << "interface client address: " << InetSocketAddress(interfaces01.GetAddress(0), serverPort) << std::endl;
    std::cout << "interface server address: " << InetSocketAddress(interfaces01.GetAddress(1), serverPort) << std::endl;

    app->Setup(serverPort, InetSocketAddress(interfaces12.GetAddress(1), port));
    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(30.0));
    nodes.Get(1)->AddApplication(app);

    AnimationInterface anim("udp-animation.xml");  // 动画文件名
    anim.SetConstantPosition(nodes.Get(0), 10, 50); // 0号节点位置
    anim.SetConstantPosition(nodes.Get(1), 50, 50); // 1号节点位置
    anim.SetConstantPosition(nodes.Get(2), 90, 50); // 2号节点位置

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
