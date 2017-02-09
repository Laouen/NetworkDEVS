//CPP:NetworkDEVS/switch_protocol.cpp
#if !defined switch_protocol_h
#define switch_protocol_h

#include "layer.h"

class switch_protocol : public Layer<int, int, link::Frame, int, int, int, link::Multiplexed_frame, int> {

  std::map<MAC,ushort> forwarding_table;
  ushort interface_amount;

  /************** times ***************/
  double process_frame_time = 0.001;

  /********** switch methods **********/
  void processFrame(link::Frame&);
  void sendToAllInterfaces(link::Frame&);
  void send(link::Frame& frame, ushort interface);

public:
  switch_protocol(const char *n): Layer(n) {};
  void init(double, ...);
  double ta(double);
  Event lambda(double);
  void exit() {}

  virtual void dinternal(double);
  virtual void dexternal(double) {};
};

#endif