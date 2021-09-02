/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//        n0     n1
//        |      | 
//       ----------
//       | Switch |
//       ----------
//        |      | 
//        n2     n3
//
//
// - CBR/UDP flows from n0 to n1 and from n3 to n0
// - DropTail queues 
// - Tracing of queues and packet receptions to file "csma-bridge.tr"

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Assignment_1");

static void
Rxtime (std::string context, Ptr<const Packet> p, const Address &a)
{
	static double bytes1, bytes2, bytes3=0;
	if (context == "Flow1"){
		bytes1 += p->GetSize();
		NS_LOG_UNCOND("1\t" << Simulator::Now().GetSeconds() << "\t" << bytes1*8/1000000/(Simulator::Now().GetSeconds()-1));
	} else if (context == "Flow2"){
		bytes2 += p->GetSize();
		NS_LOG_UNCOND("2\t" << Simulator::Now().GetSeconds() << "\t" << bytes2*8/1000000/(Simulator::Now().GetSeconds()-3));
	} else if (context == "Default"){
		bytes3 += p->GetSize();
		NS_LOG_UNCOND("3\t" << Simulator::Now().GetSeconds() << "\t" << bytes3*8/1000000/(Simulator::Now().GetSeconds()-1));
	}
}

int 
main (int argc, char *argv[])
{
  //
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //
#if 0 
  LogComponentEnable ("CsmaBridgeExample", LOG_LEVEL_INFO);
#endif

  //
  // Allow the user to override any of the defaults and the above Bind() at
  // run-time, via command-line arguments
  //
  CommandLine cmd;
  cmd.Parse (argc, argv);

  //
  // Explicitly create the nodes required by the topology (shown above).
  //
  NS_LOG_INFO ("Create nodes.");
  NodeContainer terminals;
  terminals.Create (4);

  NodeContainer csmaSwitch;
  csmaSwitch.Create (1);

  NS_LOG_INFO ("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (10)));

  // Create the csma links, from each terminal to the switch

  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;

  for (int i = 0; i < 4; i++)
    {
      NetDeviceContainer link = csma.Install (NodeContainer (terminals.Get (i), csmaSwitch));
      terminalDevices.Add (link.Get (0));
      switchDevices.Add (link.Get (1));
    }

  // Create the bridge netdevice, which will do the packet switching
  Ptr<Node> switchNode = csmaSwitch.Get (0);
  BridgeHelper bridge;
  bridge.Install (switchNode, switchDevices);

  // Add internet stack to the terminals
  InternetStackHelper internet;
  internet.Install (terminals);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  //
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (terminalDevices);

  //
  // Create an OnOff application to send UDP datagrams from node 2 to node 1.
  //
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
	
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (Ipv4Address ("10.1.1.2"), port+1)));
	onoff.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
  onoff.SetConstantRate (DataRate ("10Mb/s"));

  ApplicationContainer app = onoff.Install (terminals.Get (2));
  // Start the application
  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));

  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port+1)));
  ApplicationContainer sinkapp = sink.Install (terminals.Get (1));
  sinkapp.Start (Seconds (0.0));
	sinkapp.Get(0)->TraceConnect("Rx", "Default", MakeCallback(&Rxtime));
	
  // 
  // Create a similar flow from n0 to n1, starting at time 1.0 seconds
  //
	
	OnOffHelper onoff1 ("ns3::UdpSocketFactory",
			Address (InetSocketAddress (Ipv4Address ("10.1.1.2"), port)));

  //onoff.SetAttribute ("Remote", 
//                      AddressValue (InetSocketAddress (Ipv4Address ("10.1.1.1"), port)));
	onoff1.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
  onoff1.SetConstantRate (DataRate ("5Mb/s"));
  ApplicationContainer app1 = onoff1.Install (terminals.Get (0));
  app1.Start (Seconds (1.0));
  app1.Stop (Seconds (10.0));

	PacketSinkHelper sink1 ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  ApplicationContainer sinkapp1 = sink1.Install (terminals.Get (1));
	sinkapp1.Start (Seconds (0.0));
	sinkapp1.Get(0)->TraceConnect("Rx", "Flow1", MakeCallback (&Rxtime));

	/*OnOffHelper onoff2 ("ns3::UdpSocketFactory",
                     Address (InetSocketAddress (Ipv4Address ("10.1.1.3"), port)));*/

	onoff1.SetAttribute ("Remote",
                    AddressValue (InetSocketAddress (Ipv4Address ("10.1.1.1"), port)));
	onoff1.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
  onoff1.SetConstantRate (DataRate ("10Mb/s"));

  ApplicationContainer app2 = onoff1.Install (terminals.Get (3));
  app2.Start (Seconds (3.0));
  app2.Stop (Seconds (13.0));

  PacketSinkHelper sink2 ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  ApplicationContainer sinkapp2 = sink1.Install (terminals.Get (0));
  sinkapp2.Start (Seconds (0.0));
  sinkapp2.Get(0)->TraceConnect("Rx", "Flow2", MakeCallback (&Rxtime));


  NS_LOG_INFO ("Configure Tracing.");

  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "csma-bridge.tr"
  //
  //AsciiTraceHelper ascii;
  //csma.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge.tr"));

  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     csma-bridge-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  csma.EnablePcapAll ("Assignment_1", false);

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop(Seconds(15));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
