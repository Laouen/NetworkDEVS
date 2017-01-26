//CPP:NetworkDEVS/swp_protocol.cpp
#if !defined swp_protocol_h
#define swp_protocol_h

#include "layer.h"

// there is no control type for lower layer physical protocols
class swp_protocol : public Layer<link::Frame, ip::arp::Packet, link::Frame, int> {

  std::queue<link::Frame> akcToSend;
  std::queue<link::Frame> readyToDeliver;
  swp::SwpState<link::Frame> state;
  MAC mac;

  /********** TIMES *****************/
  double send_swp_time = 0.001;
  double deliver_swp_time = 0.001;
  double swp_timeout_time = 0.001;

public:
  swp_protocol(const char *n): Layer(n) {};
  void init(double, ...);
  double ta(double t);
  Event lambda(double);
  void exit() {}

  virtual void dinternal(double t);
};

#endif