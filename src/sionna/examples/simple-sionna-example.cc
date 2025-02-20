#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/sionna-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteRandomMobilityAndTrafficWithSionna");

void PositionChanged(Ptr<const MobilityModel> mobility) {
    Vector pos = mobility->GetPosition();
    NS_LOG_UNCOND("Node " << mobility->GetObject<Node>()->GetId() 
                 << " moved. New position is: x=" << pos.x 
                 << ", y=" << pos.y);
}

int main(int argc, char* argv[])
{
    double txPower = 0;  // dBm
    uint16_t earfcn = 100;  // EARFCN (2100 MHz)
    
    bool sionna = true; // Use Sionna
    std::string server_ip = "127.0.0.1"; //localhost
    bool local_machine = true;
    bool verb = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("txPower", "TX power in dBm", txPower);
    cmd.AddValue("earfcn", "EARFCN (default 100 - 2.1GHz)", earfcn);
    // Command Line Parameters for Sionna
    cmd.AddValue ("sionna", "Use and enable Sionna as channel", sionna); 
    cmd.AddValue ("sionna-server-ip", "Sionna server IP Address", server_ip); 
    cmd.AddValue ("sionna-local-machine", "Set to True if Sionna is running locally", local_machine);
    cmd.AddValue ("sionna-verbose", "Enable verbose logging to compare values from Sionna and ns-3 channel model", verb);
    cmd.Parse(argc, argv);

    // Create SionnaHelper
    SionnaHelper& sionnaHelper = SionnaHelper::GetInstance();

    if (sionna)
    {
        sionnaHelper.SetSionna(sionna); // Enable Sionna
        sionnaHelper.SetServerIp(server_ip); // Set server IP (in this case, localhost)
        sionnaHelper.SetLocalMachine(local_machine); // Set True if Sionna is running locally, as in this example
        sionnaHelper.SetVerbose(verb); // Enable verbose logging
    }

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);
    lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(earfcn));
    lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(earfcn + 18000));

    NodeContainer enbNodes;
    NodeContainer ueNodes;
    enbNodes.Create(1);
    ueNodes.Create(3);

    // Introduce mobility for eNB (static) and UEs (random walk, for example)
    MobilityHelper mobilityEnb;
    mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityEnb.Install(enbNodes);

    MobilityHelper mobilityUe;
    mobilityUe.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                    "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                    "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobilityUe.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Mode", StringValue("Time"),
                                "Time", StringValue("2s"),
                                "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=5.0]"),
                                "Bounds", RectangleValue(Rectangle(0, 100, 0, 100)));
    mobilityUe.Install(ueNodes);

    // Logs for position changes
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i) {
        Ptr<Node> node = ueNodes.Get(i);
        node->GetObject<MobilityModel>()->TraceConnectWithoutContext("CourseChange",
                                                                    MakeCallback(&PositionChanged));
    }

    // Install Internet stack on UEs
    InternetStackHelper internet;
    internet.Install(ueNodes);

    // Create and install LTE devices
    NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueDevs = lteHelper->InstallUeDevice(ueNodes);

    // Set TX power for eNB and UEs
    enbDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(txPower);
    for (uint32_t i = 0; i < ueDevs.GetN(); ++i) {
        ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetPhy()->SetTxPower(txPower);
    }

    // Attach UEs to eNB
    for (uint32_t i = 0; i < ueNodes.GetN(); i++) {
        lteHelper->Attach(ueDevs.Get(i), enbDevs.Get(0));
    }

    // Generate UDP traffic
    uint16_t port = 9;
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i) {
        UdpEchoServerHelper echoServer(port);
        ApplicationContainer serverApps = echoServer.Install(ueNodes.Get(i));
        serverApps.Start(Seconds(1.0));
        serverApps.Stop(Seconds(20.0));

        for (uint32_t j = 0; j < ueNodes.GetN(); ++j) {
            if (i != j) {
                UdpEchoClientHelper echoClient(Ipv4Address("7.0.0.1"), port);
                echoClient.SetAttribute("MaxPackets", UintegerValue(10));
                echoClient.SetAttribute("Interval", TimeValue(Seconds(CreateObject<UniformRandomVariable>()->GetValue(1.0, 3.0))));
                echoClient.SetAttribute("PacketSize", UintegerValue(1024));

                ApplicationContainer clientApps = echoClient.Install(ueNodes.Get(j));
                clientApps.Start(Seconds(2.0 + j));
                clientApps.Stop(Seconds(20.0));
            }
        }
    }
    
    Simulator::Stop(Seconds(2.0));
    lteHelper->EnablePhyTraces();
    lteHelper->EnableMacTraces();
    lteHelper->EnableRlcTraces();

    Simulator::Run();
    Simulator::Destroy();

    // Optionally, close Sionna on server
    sionnaHelper.ShutdownSionna();

    return 0;
}