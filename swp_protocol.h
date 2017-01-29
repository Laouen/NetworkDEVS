//CPP:NetworkDEVS/swp_protocol.cpp
#if !defined swp_protocol_h
#define swp_protocol_h

#include "layer.h"

// there is no control type for lower layer physical protocols
class swp_protocol : public Layer<ip::Packet, link::Control, link::Frame, int> {

  std::queue<link::Frame> framesToSend;

  std::list<link::arp::Entry> ARPTable;
  
  swp::SwpState<link::Frame> state;
  MAC mac;
  IPv4 ip;
  ushort interface;

  /********** TIMES *****************/
  double send_swp_time = 0.001;
  double deliver_swp_time = 0.001;
  double swp_timeout_time = 0.001;
  double deliver_up_layer_time = 0.001;
  double process_arp_time = 0.001;
  double process_link_control_time = 0.001;

  static bool SeqnoRWSComparator(swp::SwpSeqno, swp::SwpSeqno);
  static bool recvQ_slotComparator(swp::recvQ_slot<link::Frame>, swp::recvQ_slot<link::Frame>);

  void processLinkControl(link::Control control, double t);
  void sendSWP(link::Frame frame, double t);
  void deliverSWP(link::Frame frame, double t);
  bool sendWindowsIsFull();
  void storeSwpHdr(const swp::SwpHdr& hdr, unsigned long& preamble);
  link::Frame wrapInFrame(const ip::Packet& packet, MAC dest_mac);
  link::Frame wrapInFrame(const ip::arp::Packet& packet);
  bool validMAC(MAC MAC_destination);
  bool swpInWindow(swp::SwpSeqno startNum, ushort WS, swp::SwpSeqno seqNum);
  void sendAck(MAC mac_dest, double t);
  void loadSwpHdr(swp::SwpHdr& hdr, const unsigned long& preamble);
  void swpTimeout(double t);
  void updateTimeouts(double t);
  double nexTimeout();
  bool timeoutTriggered();
  ip::Packet getIpPacket(const link::Frame& frame);
  link::arp::Packet getARPPacket(const link::Frame& frame);
  void processReadyFrame(link::Frame frame, double t);
  void updateARPCache(double t);
  void cacheSourceMAC(MAC source_mac, IPv4 source_ip);
  void sendARPReady(IPv4 arp_ip, double t;
  void sendArpQuery(IPv4 arp_ip, double t);
  void processARPPacket(link::arp::Packet packet, double t);
  unsigned long calculateCRC();
  bool verifyCRC(link::Frame frame);

public:
  swp_protocol(const char *n): Layer(n) {};
  void init(double, ...);
  double ta(double t);
  Event lambda(double);
  void exit() {}

  virtual void dexternal(double t);
  virtual void dinternal(double t);
};

#endif