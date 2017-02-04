//CPP:NetworkDEVS/swp_protocol.cpp
#if !defined swp_protocol_h
#define swp_protocol_h

#include "layer.h"

// there is no control type for lower layer physical protocols
class swp_protocol : public Layer<ip::Packet, link::Control, link::Frame, int> {

  std::list<link::arp::Entry> ARPTable;
  swp::SwpState<link::Frame> SWPState;
  
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

  /********** SWP methods **************/

  static bool SeqnoRWSComparator(swp::SwpSeqno, swp::SwpSeqno);
  static bool recvQ_slotComparator(swp::recvQ_slot<link::Frame>, swp::recvQ_slot<link::Frame>);

  void swpSend(link::Frame frame);
  void swpDeliver(link::Frame frame);
  void swpStoreHdr(const swp::SwpHdr& hdr, unsigned long& preamble);
  void swpDeliverToUpModules(link::Frame frame);
  bool swpValidMAC(MAC MAC_destination);
  bool swpInWindow(swp::SwpSeqno startNum, ushort WS, swp::SwpSeqno seqNum);
  void swpSendAck(MAC mac_dest);
  void swpLoadHdr(swp::SwpHdr& hdr, const unsigned long& preamble);
  void swpTimeout();
  void swpUpdateTimeouts(double t);
  double swpNexTimeout();
  bool swpTimeoutTriggered();
  
  /********** Comunication methods **************/

  void sendControl(link::Ctrl c, link::Control control);
  link::Frame wrapInFrame(const ip::Packet& packet, MAC dest_mac);
  link::Frame wrapInFrame(const link::arp::Packet& packet);
  ip::Packet getIpPacket(const link::Frame& frame);
  link::arp::Packet getARPPacket(const link::Frame& frame);
  unsigned long calculateCRC();
  bool verifyCRC(link::Frame frame);
  
  /********** ARP methods **************/

  void arpCacheSourceMAC(MAC source_mac, IPv4 source_ip);
  void arpProcessPacket(link::arp::Packet packet);
  void arpUpdateCache(double t);
  void arpProcessLinkControl(link::Control control);
  bool arpCachedMAC(IPv4 dest_ip);
  MAC arpGetMAC(IPv4 dest_ip);
  void arpSendQuery(IPv4 arp_ip);

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