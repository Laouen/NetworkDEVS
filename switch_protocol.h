//CPP:NetworkDEVS/switch_protocol.cpp
#if !defined switch_protocol_h
#define switch_protocol_h

#include "template/protocol.h"
#include <set>
#include "swp_protocol.h"

class switch_protocol : public Protocol<int, int, message::Multiplexed<link::Frame>, int> {

  std::map<MAC,ushort> forwarding_table;
  std::vector<swp_protocol> swp_ports;

  MAC mac;
  ushort interface_amount;

  /************** times ***************/
  // In milliseconds
  double process_frame_time = 50;
  double send_frame_time = 5;
  double swp_timeout_time = 100;

  /********** switch methods **********/
  void processFrame(link::Frame& frame, ushort interface);
  void sendToAllInterfaces(link::Frame& frame, ushort source_interface);
  void send(link::Frame& frame, ushort interface);
  void sendFrames(ushort interface);
  void updateTimeouts(double t);
  unsigned long calculateCRC();
  bool verifyCRC(link::Frame frame);

public:
  switch_protocol(const char *n): Protocol(n) {};
  void init(double, ...);
  double ta(double);
  Event lambda(double);
  void exit() {}

  virtual void dinternal(double);
  virtual void dexternal(double);
};

#endif