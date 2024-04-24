#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/mobility-module.h"

/*
This simple test model simulates a random network with 10% of possible connections being created.
Each node(neuron) has a 50% probability of sending a spiking(Data packet) from itself. And
after receiving 4 spiking, it sends a spiking to all downstream neurons. (Loops/isolated node may exist)
*/

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("SequentialUdpCommunication");

class RespondingApp : public Application {
public:
    RespondingApp() : m_packetSize(1024), m_totalRx(0), m_threshold(4096), m_debugInterval(1.0), m_stopTime(1e15) {}
    virtual ~RespondingApp() {}

    void AddLocalPort(uint16_t localPort) {
        m_localPorts.push_back(localPort);
    }

    void AddPeerAddress(Address peerAddress) {
        m_peerAddresses.push_back(peerAddress);
//        std::cout << "Added peer address " << InetSocketAddress::ConvertFrom(peerAddress).GetIpv4() << std::endl;
    }

    void SetPacketSize(uint32_t packetSize) {
        m_packetSize = packetSize;
    }

    void SetThreshold(uint32_t threshold) {
        m_threshold = threshold;
    }

protected:
    virtual void StartApplication() override {
//        std::cout << "Starting application..." << std::endl;
        for (uint16_t port : m_localPorts) {
            Ptr<Socket> socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), port));
            socket->SetRecvCallback(MakeCallback(&RespondingApp::HandleRead, this));
            m_sockets.push_back(socket);
        }
//        std::cout << "Application started, listening on multiple ports." << std::endl;

        Simulator::Schedule(Seconds(m_debugInterval), &RespondingApp::PeriodicDebugOutput, this);
    }

    virtual void StopApplication() override {
        m_stopTime = Simulator::Now();
        for (Ptr<Socket> socket : m_sockets) {
            if (socket) {
                socket->Close();
            }
        }
        m_sockets.clear();
//        std::cout << "Application stopped." << std::endl;
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

    void PeriodicDebugOutput() {
//        std::cout << "Periodic debug output" << std::endl;
        if (Simulator::Now() + Seconds(m_debugInterval) > m_stopTime) {
//            std::cout << "Current time is " << Simulator::Now().GetSeconds() << "s" << std::endl;
//            std::cout << "Stop time is " << m_stopTime.GetSeconds() << "s" << std::endl;
//            std::cout << "Stop time reached, stopping debug output." << std::endl;
            return;
        }

        Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
        if (uv->GetValue() < 0.5) {  // 50% chance to output a debug message
            Time now = Simulator::Now();  // Get the current simulation time
//            std::cout << "Time " << now.GetSeconds() << "s: Periodic debug - Total received "
//                      << m_totalRx << " bytes." << std::endl;
            }
            SendFeedback();
            Simulator::Schedule(Seconds(m_debugInterval), &RespondingApp::PeriodicDebugOutput, this);
    }

private:
    std::vector<Ptr<Socket>> m_sockets;  // 网络套接字列表
    std::vector<uint16_t> m_localPorts;  // 本地端口列表
    std::vector<Address> m_peerAddresses;  // 远端地址列表
    uint32_t m_packetSize;  // 数据包大小
    uint32_t m_totalRx;  // 总接收字节
    uint32_t m_threshold;  // 触发反馈的阈值
    double m_debugInterval;  // Interval for debug output
    Time m_stopTime;  // Application stop time
};

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    uint32_t numNodes = 20;  // 网络中的节点数量
    double linkProbability = 0.1;  // 节点间建立连接的概率
    uint16_t receiver_port = 9;  // 所有节点监听的统一端口

    NodeContainer nodes;
    nodes.Create(numNodes);
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");

    std::vector<std::vector<Ipv4Address>> peerAddresses(numNodes);  // 存储每个节点的对等地址

    int subnet = 1;
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();

    for (uint32_t i = 0; i < numNodes; ++i) {
        for (uint32_t j = i + 1; j < numNodes; ++j) {
            if (rand->GetValue() <= linkProbability) {
                NetDeviceContainer devices = pointToPoint.Install(nodes.Get(i), nodes.Get(j));
                std::ostringstream subnetAddress;
                subnetAddress << "10.1." << subnet++ << ".0";
                address.SetBase(subnetAddress.str().c_str(), "255.255.255.0");
                Ipv4InterfaceContainer iface = address.Assign(devices);
                peerAddresses[i].push_back(iface.GetAddress(1));  // 存储j节点的地址到i的列表
                peerAddresses[j].push_back(iface.GetAddress(0));  // 存储i节点的地址到j的列表
            }
        }
    }

    // 为每个节点安装并配置RespondingApp
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<RespondingApp> app = CreateObject<RespondingApp>();
        app->AddLocalPort(receiver_port);
        for (const auto &peerAddress : peerAddresses[i]) {
            app->AddPeerAddress(InetSocketAddress(peerAddress, receiver_port));
        }
        app->SetStartTime(Seconds(1.0));
        app->SetStopTime(Seconds(30.0));
        nodes.Get(i)->AddApplication(app);
    }


//    // Install RespondingApp on all nodes
//    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
//        Ptr<RespondingApp> app = CreateObject<RespondingApp>();
//        app->AddLocalPort(receiver_port);
//
//        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
//        Ipv4Address ipAddr = ipv4->GetAddress(1, 0).GetLocal();
//
//        for (int j = 0; j < max_links; ++j) {
//            std::cout << "interfaces[j].GetAddress(0) = " << interfaces[j].GetAddress(0) << ", interfaces[j].GetAddress(1) = " << interfaces[j].GetAddress(1) << std::endl;
//            if(interfaces.size() > j && ipAddr == interfaces[j].GetAddress(0)) {
//                app->AddPeerAddress(InetSocketAddress(interfaces[j].GetAddress(1), receiver_port));
//            }
//        }
//        app->SetStartTime(Seconds(1.0));
//        app->SetStopTime(Seconds(10.0));
//
//        nodes.Get(i)->AddApplication(app);
//    }


    std::cout << "Installed RespondingApp on all nodes." << std::endl;
    //     Setup animation
    // Calculate positions for a 5x4 grid within an 80x80 area
    AnimationInterface anim("scratch/rand_send_rand_structure.xml");  // File for animation output
    int rows = 5;
    int cols = 4;
    double xSpacing = 80.0 / cols;
    double ySpacing = 80.0 / rows;
    int index = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (index < nodes.GetN()) {
                Ptr<Node> node = nodes.Get(index);
//                std::cout << "Node " << index << " at (" << j * xSpacing + xSpacing / 2 << ", "
//                          << i * ySpacing + ySpacing / 2 << ")" << std::endl;
                anim.SetConstantPosition(node, j * xSpacing + xSpacing / 2, i * ySpacing + ySpacing / 2);
                index++;
            }
        }
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
