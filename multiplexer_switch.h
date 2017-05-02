//CPP:networkDEVS/multiplexer_switch.cpp
#if !defined multiplexer_switch_h
#define multiplexer_switch_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "libs/message_queue.h"

#include "structures/abstract_types.h"
#include "structures/link.h"

class multiplexer_switch: public Simulator { 
  message::queue<link::Multiplexed_frame> _sent_frame_out;
  Event _output;

public:
  multiplexer_switch(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif