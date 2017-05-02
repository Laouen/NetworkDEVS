//CPP:networkDEVS/demultiplexer_switch.cpp
#if !defined demultiplexer_switch_h
#define demultiplexer_switch_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "libs/message_queue.h"

#include "structures/abstract_types.h"
#include "structures/link.h"

class demultiplexer_switch: public Simulator { 
  message::queue<link::Frame> _sent_frame_out;
  unsigned int _max_interface;
  Event _output;

public:
  demultiplexer_switch(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif