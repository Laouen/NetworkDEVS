//CPP:networkDEVS/demultiplexer_g.cpp
#if !defined demultiplexer_g_h
#define demultiplexer_g_h

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
#include "structures/link.h"

class demultiplexer_g: public Simulator { 
  message_list<link::Control> _sent_control_inputs;
  unsigned int _max_interface;
  Event _output;

public:
  demultiplexer_g(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif