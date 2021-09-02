#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <iostream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("week4");

int 
main(int argc, char*argv[])
{
	/*LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
	LogComponentEnable("UdpEchoClientApplication", LOG_PREFIX_TIME);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
	LogComponentEnable("week4", LOG_LEVEL_ALL);*/
	
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoClientApplication", LOG_PREFIX_TIME);
	LogComponentEnable("UdpEchoClientApplication", LOG_PREFIX_FUNC);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_PREFIX_TIME);
	LogComponentEnable("UdpEchoServerApplication", LOG_PREFIX_FUNC);
	
	NodeContainer nodes;
	nodes.Create(2);
	NodeContainer nodes2;
	nodes2.Add(nodes.Get(1));
	nodes2.Create(1);

	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue("5ms"));

	PointToPointHelper p2p_2;
	p2p_2.SetDeviceAttribute("DataRate", StringValue("0.1Mbps"));
	p2p_2.SetChannelAttribute("Delay", StringValue("10ms"));

	NetDeviceContainer devices;
	devices = p2p.Install(nodes);

	NetDeviceContainer devices2;
	devices2 = p2p_2.Install(nodes2);

	InternetStackHelper stack;
	stack.Install(nodes);
	stack.Install(nodes2.Get(1));

	Ipv4AddressHelper addr;
	addr.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = addr.Assign(devices);

	Ipv4AddressHelper addr2;
	addr2.SetBase("10.1.2.0", "255.255.255.0");
        Ipv4InterfaceContainer interfaces2 = addr2.Assign(devices2);
	
	UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
	echoClient.SetAttribute("MaxPackets", UintegerValue(1500));
	echoClient.SetAttribute("Interval", TimeValue(Seconds(0.001)));
	echoClient.SetAttribute("PacketSize", UintegerValue(1050));

	UdpEchoClientHelper echoClient2(interfaces2.GetAddress(1), 9);
        echoClient2.SetAttribute("MaxPackets", UintegerValue(1500));
        echoClient2.SetAttribute("Interval", TimeValue(Seconds(0.01)));
        echoClient2.SetAttribute("PacketSize", UintegerValue(1050));

	ApplicationContainer clientApps;
	clientApps.Add(echoClient.Install(nodes.Get(0)));
	clientApps.Start(Seconds(2.0));
	clientApps.Stop(Seconds(4.0));

	ApplicationContainer clientApps2;
        clientApps2.Add(echoClient2.Install(nodes2.Get(0)));
        clientApps2.Start(Seconds(2.0));
        clientApps2.Stop(Seconds(4.0));

	UdpEchoServerHelper echoServer(9);
	ApplicationContainer serverApps(echoServer.Install(nodes.Get(1)));
	serverApps.Start(Seconds(1));
	serverApps.Stop(Seconds(5.0));

	UdpEchoServerHelper echoServer2(9);
        ApplicationContainer serverApps2(echoServer2.Install(nodes2.Get(1)));
        serverApps2.Start(Seconds(1));
        serverApps2.Stop(Seconds(5.0));


	Simulator::Run();
	Simulator::Stop(Seconds(5.0));
	Simulator::Destroy();

	return 0;
}
