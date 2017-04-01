//CPP:NetworkDEVS/link_protocol.cpp
#if !defined link_protocol_h
#define link_protocol_h

#include "layer.h"

// there is no control type for lower layer physical protocols
class link_protocol : public Layer<ip::Datagram, link::Control, link::Frame, int> {

  std::list<link::arp::Entry> ARPTable;
  std::map<MAC,swp::State<link::Frame>> SWPTable;
  
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

  static bool SeqnoRWSComparator(swp::Seqno, swp::Seqno);
  static bool recvQ_slotComparator(swp::recvQ_slot<link::Frame>, swp::recvQ_slot<link::Frame>);

  bool swpThereIsFrameToSend(swp::State<link::Frame>*& SWPState);
  void swpSend(swp::State<link::Frame>& SWPState);
  void swpTimeout(swp::State<link::Frame>& SWPState);
  bool swpTimeoutTriggered(swp::State<link::Frame>*& SWPState);
  void swpDeliver(link::Frame frame);
  void swpStoreHdr(const swp::Hdr& hdr, unsigned long& preamble);
  bool swpInWindow(swp::Seqno startNum, ushort WS, swp::Seqno seqNum);
  void swpSendAck(MAC mac_dest, swp::Seqno LFR);
  void swpLoadHdr(swp::Hdr& hdr, const unsigned long& preamble);
  void swpUpdateTimeouts(double t);
  double swpNexTimeout();
  swp::State<link::Frame>& swpGetState(MAC mac_dest);
  
  /********** Comunication methods **************/

  void processFrame(link::Frame frame);
  bool validMAC(MAC MAC_destination);
  void sendControl(link::Ctrl c, link::Control control);
  link::Frame wrapInFrame(const ip::Datagram& packet, MAC dest_mac);
  link::Frame wrapInFrame(const link::arp::Packet& packet);
  link::Frame wrapInFrame(const link::arp::Packet& packet, MAC dest_mac);
  ip::Datagram getIpDatagram(const link::Frame& frame);
  link::arp::Packet getARPPacket(const link::Frame& frame);
  unsigned long calculateCRC();
  bool verifyCRC(link::Frame frame);
  
  /********** ARP methods **************/

  void arpCacheSourceMAC(MAC source_mac, IPv4 source_ip);
  void arpProcessPacket(link::Frame frame);
  void arpUpdateCache(double t);
  void arpProcessLinkControl(link::Control control);
  bool arpCachedMAC(IPv4 dest_ip);
  MAC arpGetMAC(IPv4 dest_ip);
  void arpSendQuery(IPv4 arp_ip);

public:
  link_protocol(const char *n): Layer(n) {};
  void init(double, ...);
  double ta(double t);
  Event lambda(double);
  void exit() {}

  virtual void dexternal(double t);
  virtual void dinternal(double t);
};

#endif