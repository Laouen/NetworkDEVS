//CPP:NetworkDEVS/swp_protocol.cpp
#if !defined swp_protocol_h
#define swp_protocol_h

#include "layer.h"

// there is no control type for lower layer physical protocols
class swp_protocol : public Layer<link::Frame, ip::arp::Packet, link::Frame, int> {

  std::queue<link::Frame> readyToDeliver;
  swp::SwpState<link::Frame> state;
  MAC mac;

  /********** TIMES *****************/
  double send_swp_time = 0.001;
  double deliver_swp_time = 0.001;
  double swp_timeout_time = 0.001;
  double deliver_up_layer_time = 0.001;


  void sendSWP(link::Frame, double);
  bool validMAC(MAC);
  bool sendWindowsIsFull();
  void storeSwpHdr(const swp::SwpHdr&, unsigned long&);
  link::Frame wrapARPInFrame(const ip::arp::Packet&);
  void deliverSWP(link::Frame, double);
  bool swpInWindow(swp::SwpSeqno, ushort, swp::SwpSeqno);
  void sendAck(MAC, double);
  void loadSwpHdr(swp::SwpHdr&, const unsigned long&);
  void swpTimeout(double);
  void updateTimeouts(double);
  double nexTimeout();
  bool timeoutTriggered();
  void deliverToUpLayer(const link::Frame&, double);
  unsigned long calculateCRC();
  bool verifyCRC(link::Frame);

  static bool SeqnoRWSComparator(swp::SwpSeqno, swp::SwpSeqno);
  static bool recvQ_slotComparator(swp::recvQ_slot<link::Frame>, swp::recvQ_slot<link::Frame>);

public:
  swp_protocol(const char *n): Layer(n) {};
  void init(double, ...);
  double ta(double t);
  Event lambda(double);
  void exit() {}

  virtual void dinternal(double t);
};

#endif