#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"
#include "ns3/double.h"
#include "ns3/boolean.h"

#include "streaming-client.h"
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StreamingClientApplication");

NS_OBJECT_ENSURE_REGISTERED (StreamingClient);

TypeId
StreamingClient::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::StreamingClient")
		.SetParent<Application> ()
		.SetGroupName("Applications")
		.AddConstructor<StreamingClient> ()
		.AddAttribute ("RemotePort", "Port on which we listen for incoming packets.",
									 UintegerValue (9),
									 MakeUintegerAccessor (&StreamingClient::m_port),
									 MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&StreamingClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("PacketSize", 
                   "The packet size",
									 UintegerValue (100),
									 MakeUintegerAccessor (&StreamingClient::m_packetSize),
									 MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BufferSize", 
                   "The frame buffer size",
									 UintegerValue (40),
									 MakeUintegerAccessor (&StreamingClient::m_bufferSize),
									 MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("PauseSize", 
                   "The pause size for the frame buffer",
									 UintegerValue (30),
									 MakeUintegerAccessor (&StreamingClient::m_pause),
									 MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ResumeSize", 
                   "The resume size for the frame buffer",
									 UintegerValue (5),
									 MakeUintegerAccessor (&StreamingClient::m_resume),
									 MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("FramePackets", 
                   "# of packets in forms of a frame",
                   UintegerValue (100),
                   MakeUintegerAccessor (&StreamingClient::m_fpacketN),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ConsumeStartTime", 
                   "Consumer Start Time",
                   DoubleValue (1.1),
                   MakeDoubleAccessor (&StreamingClient::m_consumeTime),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PacketLossEnable", 
                   "Forced Packet Loss on/off",
                   BooleanValue (false),
                   MakeBooleanAccessor (&StreamingClient::m_lossEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("ErrorRate", 
                   "ErrorRate",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&StreamingClient::m_errorRate),
                   MakeDoubleChecker<double> ())
		.AddAttribute ("BufferingSize",
                   "BufferingSize",
                   UintegerValue (25),
                   MakeUintegerAccessor (&StreamingClient::m_bufsize),
                   MakeUintegerChecker<uint32_t> ())
		;
	return tid;
}

StreamingClient::StreamingClient ()
{
	NS_LOG_FUNCTION (this);
	m_seqNumber = 0;
	m_consumEvent = EventId ();
	m_genEvent = EventId ();
	m_frameCnt = 0;
	m_frameIdx = 0;
	cur_seqnum = 0;
	start = false;
}

StreamingClient::~StreamingClient ()
{
	NS_LOG_FUNCTION (this);
	m_socket = 0;
}

uint32_t sum1 = 0;
uint32_t sum2 = 0;
bool flag = false;

void
StreamingClient::FrameConsumer (void)
{
	// Frame Consume
	if (m_frameCnt >= 0 && start)
	{
		if(!flag){
			NS_LOG_INFO("Start Consuming");
			flag = true;
		}
		if (m_frameBuffer.find (m_frameIdx) != m_frameBuffer.end() )
		{
			m_frameCnt -= 1;
			m_frameBuffer.erase(m_frameIdx);
			NS_LOG_INFO("FrameConsumerLog::Consume " << m_frameIdx);
			sum1++;
		}
		else if (m_frameCnt == 0)
		{
			NS_LOG_INFO("FrameConsumerLog::NoConsume " << m_frameIdx);
			sum2++;
		}
		else
		{
			NS_LOG_INFO("FrameConsumerLog::NoConsume " << m_frameIdx);
			sum2++;
		}
		double val = (double) sum1 / (double) (sum1 + sum2);
		NS_LOG_INFO("FrameConsumerLog::RemainFrames: " << m_frameCnt);
		NS_LOG_INFO("Frame Consume Rate: " << val << " Consume num: " << sum1);
		m_frameIdx += 1;
	}
	else if (m_frameCnt < 0)
	{
		NS_LOG_INFO("FrameCountError!");
		exit (1);
	}
	//m_frameIdx += 1;

	// FrameBufferCheck
	if (m_frameCnt >= (int)m_pause)
	{
		Ptr<Packet> p;
		p = Create<Packet> (m_packetSize);
		SeqTsHeader header;
		header.SetSeq (2);
		p->AddHeader (header);

		Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
		udpSocket->SendTo (p, 0, m_peerAddress);
	}
	else if (m_frameCnt <= (int)m_resume)
	{
		uint32_t val = 0;
		if(m_frameCnt < m_bufsize) val = 1;
		Ptr<Packet> p;
		p = Create<Packet> (m_packetSize);
		SeqTsHeader header;
		header.SetSeq (val);
		p->AddHeader (header);

		Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
		udpSocket->SendTo (p, 0, m_peerAddress);
	}
	else if(m_frameCnt >= m_bufsize){
		Ptr<Packet> p;
		p = Create<Packet> (m_packetSize);
    SeqTsHeader header;
    header.SetSeq (3);
    p->AddHeader (header);

    Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
    udpSocket->SendTo (p, 0, m_peerAddress);
	}

	m_consumEvent = Simulator::Schedule ( Seconds ((double)1.0/60), &StreamingClient::FrameConsumer, this);
}


void 
StreamingClient::FrameGenerator (void)
{
	if (m_frameCnt < (int)m_bufferSize)
	{
		std::map<uint32_t,FrameCheck>::iterator iter;

		for (iter = m_pChecker.begin(); iter != m_pChecker.end();)
		{
			uint32_t idx = iter->first;

			if (idx < m_frameIdx)
			{
				m_pChecker.erase(iter++);
			}
			else
			{
				uint32_t count = 0;
				for (uint32_t i=0; i<m_fpacketN; i++)
				{
					FrameCheck c = iter->second;
					if (c.c[i] == 1)
						count++;
				}

				if (count == m_fpacketN)
				{
					m_pChecker.erase(iter++);

					m_frameCnt++;
					m_frameBuffer.insert({idx, true});
					if(!start && m_frameCnt >= m_bufsize) start = true;
				}
				else
				{
					++iter;
				}
			}
		}
	}

	m_genEvent = Simulator::Schedule ( Seconds ((double)1.0/20), &StreamingClient::FrameGenerator, this);
}


void 
StreamingClient::StartApplication (void)
{
	NS_LOG_FUNCTION (this);
	
	if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
    if (m_socket->Bind (local) == -1)
    {
			NS_FATAL_ERROR ("Failed to bind socket");
    }
    if (addressUtils::IsMulticast (m_local))
		{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
      if (udpSocket)
      {
        // equivalent to setsockopt (MCAST_JOIN_GROUP)
        udpSocket->MulticastJoinGroup (0, m_local);
      }
      else
      {
        NS_FATAL_ERROR ("Error: Failed to join multicast group");
      }
    }
  }
	cur_seqnum = 0;
	m_socket->SetRecvCallback (MakeCallback (&StreamingClient::HandleRead, this));
	m_consumEvent = Simulator::Schedule ( Seconds (m_consumeTime), &StreamingClient::FrameConsumer, this);
	FrameGenerator ();
}

void
StreamingClient::StopApplication ()
{
	NS_LOG_FUNCTION(this);
	if (m_socket != 0) 
  {
		m_socket->Close ();
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }
	Simulator::Cancel (m_consumEvent);
}

void 
StreamingClient::HandleRead (Ptr<Socket> socket)
{
	NS_LOG_FUNCTION (this << socket);

	Ptr<Packet> packet;
	Address from;
	Address localAddress;
	while ((packet = socket->RecvFrom (from)))
	{
		socket->GetSockName (localAddress);
		from_addr = from;
		// Packet Log
		/*
		if (InetSocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
					InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
					InetSocketAddress::ConvertFrom (from).GetPort ());
		}
		*/

		if (m_lossEnable)
		{
			double prob = (double)rand() / RAND_MAX;
			if (prob <= m_errorRate)
				continue;
		}

		SeqTsHeader seqTs;
		packet->RemoveHeader (seqTs);
		uint32_t seqN = seqTs.GetSeq();
		//NS_LOG_INFO("Get seqN " << seqN << " " << cur_seqnum);
		// Loss control
		if(seqN == cur_seqnum) cur_seqnum++;
		else if(seqN > cur_seqnum){
			NS_LOG_INFO("Detect Loss ON " << cur_seqnum << " ~ " << seqN-1);
			for(uint32_t i=cur_seqnum; i<seqN; i++){
				//NS_LOG_INFO("Detect Loss ON " << i);
				Ptr<Packet> pkt;
				pkt = Create<Packet> (m_packetSize);
				SeqTsHeader lossTs;
				lossTs.SetSeq (i+10);
				pkt->AddHeader(lossTs);
				socket->SendTo(pkt, 0, from);
				loss_q.push(i);
			}
			cur_seqnum = seqN + 1;
		}
		else{
			if(seqN != 0) NS_LOG_INFO("Received loss packet " << seqN);
			while(!loss_q.empty()){
				if(seqN == loss_q.front()){
					loss_q.pop();
					break;
				}
				else if(seqN == 0) break;
				else{
					uint32_t val = loss_q.front();
					loss_q.pop();
					if(val/100 >= m_frameIdx){
						Ptr<Packet> pkt;
						pkt = Create<Packet> (m_packetSize);
						SeqTsHeader lossTs;
						lossTs.SetSeq (val+10);
						pkt->AddHeader(lossTs);
						socket->SendTo(pkt, 0, from);
						loss_q.push(val);
						
						NS_LOG_INFO("Resend loss packet " << val);
					}
				}

			}
		}

		uint32_t frameIdx = seqN/m_fpacketN;
		seqN = seqN - frameIdx * m_fpacketN;

		//std::cout << "Frame Idx: " << frameIdx << "/ Seq: " << seqN << std::endl;

		if (m_pChecker.size() < (m_bufferSize * 2 * m_fpacketN) )
		{
			if (m_pChecker.find(frameIdx) != m_pChecker.end())
			{
				FrameCheck tc = m_pChecker[frameIdx];
				tc.c[seqN] = 1;
				m_pChecker[frameIdx]=tc;
			}
			else
			{
				FrameCheck c;
				c.c[seqN] = 1;
				m_pChecker.insert({frameIdx,c});
			}
		}
	}
}

FrameCheck::FrameCheck ()
{
	for (int i=0; i<100; i++)
		c[i] = 0;
}

FrameCheck::~FrameCheck()
{
}

}
