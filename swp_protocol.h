#if !defined swp_protocol_h
#define swp_protocol_h

#include "template/protocol.h"

class swp_protocol {
private:
	// Logger
  	Logger logger;

  	MAC mac;
	swp::State<link::Frame> swp_state;

	static bool SeqnoRWSComparator(swp::Seqno a, swp::Seqno b);
	static bool recvQ_slotComparator(swp::recvQ_slot<link::Frame> a, swp::recvQ_slot<link::Frame> b);
	void storeHdr(const swp::Hdr& hdr, unsigned long& preamble);
	void sendAck(MAC mac_dest, swp::Seqno LFR);
	bool inWindow(swp::Seqno startNum, ushort WS, swp::Seqno seqNum);
	void loadHdr(swp::Hdr& hdr, const unsigned long& preamble);

public:
	std::queue<link::Frame> accepted_frames;
	std::queue<link::Frame> to_send_frames;
	
	/************ Times ******************/
	// Time in milliseconds
	double send_time = 5; 
	double infinity = std::numeric_limits<double>::max();
	
	swp_protocol();

	void init(MAC other_mac, std::string module_name);

	bool thereIsFrameToSend();
	bool timeoutTriggered();
	void timeout();
	double nexTimeout();
	void updateTimeouts(double t);

	// send/receive main methods
	void sendFrame(link::Frame frame);
	void send();
	void processFrame(link::Frame frame);

	void setModuleName(std::string module_name);
};

#endif