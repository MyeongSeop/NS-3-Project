#ifndef STREAMING_STREAMER_H
#define STREAMING_STREAMER_h

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include <queue>

namespace ns3 {

class Socket;
class Packet;

class StreamingStreamer : public Application
{
public:
	static TypeId GetTypeId (void);
	StreamingStreamer ();
	virtual ~StreamingStreamer ();
	
	void SetDataSize (uint32_t dataSize);
	uint32_t GetDataSize (void) const;
private:
	virtual void StartApplication (void);
	virtual void StopApplication (void);

	void ScheduleTx (Time dt);
	void ScheduleLoss (Time dt);
	void SendPacket (void);
	void HandleRead (Ptr<Socket> socket);

	void GenerateBuf (void);

	uint32_t m_size;
	
	uint32_t m_sent;
	Ptr<Socket> m_socket;
	Address m_peerAddress;
	uint16_t m_peerPort;
	EventId m_sendEvent;

	uint32_t m_seqNumber;
	uint32_t m_fps;
	uint32_t m_fpacketN;
	bool m_pause;

	bool m_lossEnable;
	double m_errorRate;

	//Loss control
	std::queue<uint32_t> loss_pk;
	bool m_loss;

	//Buffering
	uint32_t m_bufsize;
	std::queue<uint32_t> send_buf;
	EventId m_genEvent;
	bool m_boost;
	bool m_gogo;
};

}

#endif
