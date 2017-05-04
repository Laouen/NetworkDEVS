//CPP:NetworkDEVS/switch_protocol.cpp
#if !defined switch_protocol_h
#define switch_protocol_h

#include "template/layer.h"
#include <set>
#include "swp_protocol.h"

class switch_protocol : public Layer<int, int, message::Multiplexed<link::Frame>, int> {

  std::map<MAC,ushort> forwarding_table;
  std::vector<swp_protocol> swp_ports;

  MAC mac;
  ushort interface_amount;

  /************** times ***************/
  double process_frame_time = 0.001;
  double send_frame_time = 0.001;
  double swp_timeout_time = 0.001;

  /********** switch methods **********/
  void processFrame(link::Frame& frame, ushort interface);
  void sendToAllInterfaces(link::Frame& frame, ushort source_interface);
  void send(link::Frame& frame, ushort interface);
  void sendFrames(ushort interface);
  void updateTimeouts(double t);
  unsigned long calculateCRC();
  bool verifyCRC(link::Frame frame);

public:
  switch_protocol(const char *n): Layer(n) {};
  void init(double, ...);
  double ta(double);
  Event lambda(double);
  void exit() {}

  virtual void dinternal(double);
  virtual void dexternal(double);
};

#endif