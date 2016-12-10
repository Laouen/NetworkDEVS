//CPP:networkDEVS/demultiplexer.cpp
#if !defined demultiplexer_h
#define demultiplexer_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "libs/message_list.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"

class demultiplexer: public Simulator { 
  message_list<udp::Control> _sent_inputs;
  int _max_id;
  Event _output;

public:
  demultiplexer(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif