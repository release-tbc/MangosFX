#include <Log.h>
#include <Util.h>
#include <WorldPacket.h>
#include <cPacketOpcodes.h>
#include <cIncludes.h>

#include "ClusterOpcodes.h"
#include "ClusterSession.h"

ClusterSession::ClusterSession()
{
	m_addr = "<error>";
	mustStop = false;
}

ClusterSession::~ClusterSession()
{
	m_sock->Close();
	sLog.outBasic("Closing connection with %s and Destroying Thread",m_addr.c_str());
	///- empty incoming packet queue
    WorldPacket* packet;
    while(_recvQueue.next(packet))
        delete packet;
}

void ClusterSession::QueuePacket(WorldPacket* new_packet)
{
    _recvQueue.add(new_packet);
}

void ClusterSession::SetParams(SocketTCP* sock, std::string addr)
{
	m_sock = sock;
	m_addr = addr;
}

void ClusterSession::run()
{
	SendClusterIdentity();
	while(!mustStop)
	{
		Packet pkt;
		Socket::Status st = m_sock->Receive(pkt);
		if(CheckState(st))
			HandlePacket(&pkt);
		Update();
		ACE_Based::Thread::Sleep(100);
	}
	m_sock->Close();
	sLog.outBasic("Closing connection with %s",m_addr.c_str());
}

bool ClusterSession::CheckState(Socket::Status st)
{
	switch(st)
	{
		case 3 /*Socket::Status::Error*/:
			error_log("Socket Error for %s",m_addr.c_str());
			mustStop = true;
			return false;
		case 2 /*Socket::Status::Disconnected*/:
			error_log("Link with %s lost...",m_addr.c_str());
			mustStop = true;
			return false;
		case 0 /*Socket::Done)*/:
			return true;
		case 1 /*Socket::NotReady*/:
			//error_log("Socket isn't ready !");
			return false;
	}
	return true;
}

void ClusterSession::HandlePacket(Packet* pck)
{
	if(pck->GetDataSize() < 2)
	{
		error_log("Packet size for Cluster is wrong...");
		return;
	}

	// Get opcode
	uint16 opcode = 0;
	*pck >> opcode;
	if(!opcode)
		return;

	if(opcode >= MAX_C_OPCODES)
	{
		error_log("Cluster receive unhandled opcode %u",opcode);
		return;
	}

	// Recopy data into WorldPacket
	WorldPacket packet(opcode);
	for(uint32 i=2;i<pck->GetDataSize();i++)
		packet << pck->GetData()[i];

	packet.hexlike();
	// Pointer to keep data alive
	WorldPacket* pkt = new WorldPacket(packet);
	QueuePacket(pkt);
}

void ClusterSession::SendClusterIdentity()
{
	Packet pkt;
	pkt << Uint16(C_SMSG_CLUSTER_TYPE);
	pkt << Uint8(C_BG);
	SendPacket(&pkt);
}

void ClusterSession::SendPing()
{
	Packet pkt;
	pkt << Uint16(C_SMSG_PING) << Uint8(urand(0,5));
	SendPacket(&pkt);
}

void ClusterSession::SendPacket(const Packet* pck)
{
	if(!m_sock || !m_sock->IsValid())
		return;
	uint16 opcode;
	&pck >> opcode;

	error_log("size %u opcode %u",pck->GetDataSize(),opcode);
	Socket::Status st = m_sock->Send((Packet&)*pck);
	//m_sock->SetBlocking(false);
	CheckState(st);
}

void ClusterSession::SendNullPacket()
{
	Packet pkt;
	pkt << uint16(C_CMSG_NULL);
	SendPacket(&pkt);
}

void ClusterSession::SendUint32(uint32 value)
{
	Packet pkt;
	pkt << uint16(C_SMSG_GET_UINT32) << uint32(value);
	SendPacket(&pkt);
}

void ClusterSession::Update()
{
	WorldPacket* packet;
	while (_recvQueue.next(packet) && m_sock && m_sock->IsValid())
    {
		PacketOpcodeHandler opHandle = PckOpH[packet->GetOpcode()];
		error_log("Packet %u recv",packet->GetOpcode());
		try
		{
			(this->*opHandle.handler)(*packet);
		}
		catch(ByteBufferException &)
        {
            if(sLog.IsOutDebug())
            {
                sLog.outDebug("Dumping error causing packet:");
                packet->hexlike();
            }
        }
		catch(std::exception &e)
		{
			error_log("FATAL STD ERROR : %s",e.what());
		}
		delete packet;
	}
}

void ClusterSession::Handle_ClusterPing(WorldPacket &pck)
{
	sLog.outDebug("C_CMSG_PING received...");
	uint8 ping;
	pck >> ping;
	Packet packet;
	packet << uint16(C_SMSG_PING_RESP);
	packet << uint8(ping);
	SendPacket(&packet);
}

void ClusterSession::SendMonoPlayerPacket(uint64 guid,WorldPacket &pck)
{
	Packet pkt;
	pkt << uint64(guid);
	pkt << uint16(pck.size());
	pkt << uint16(pck.GetOpcode());
	for(uint16 i=0;i<pck.size();i++)
	{	
		uint8 tmp;
		pck >> tmp;
		pkt << uint8(tmp);
	}
	// todo : verify packet to transmit
	error_log("Packet size %u",pkt.GetDataSize());
	SendPacket(&pkt);
}

void ClusterSession::SendMultiPlayerPacket(std::vector<uint64> GUIDs,WorldPacket &pck)
{
	Packet pkt;
	pkt << uint16(pck.size());
	pkt << uint16(pck.GetOpcode());

	for(uint16 i=0;i<pck.size();i++)
	{	
		uint8 tmp;
		pck >> tmp;
		pkt << uint8(tmp);
	}

	pkt << uint16(GUIDs.size());
	for(std::vector<uint64>::const_iterator itr = GUIDs.begin();itr != GUIDs.end();++itr)
		pkt << uint64(*itr);

	// todo : verify packet to transmit
	error_log("Packet size %u",pkt.GetDataSize());
	SendPacket(&pkt);
}