//CPP:networkDEVS/demultiplexer_ip_link.cpp
#if !defined demultiplexer_ip_link_h
#define demultiplexer_ip_link_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "libs/message_queue.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"
#include "structures/link.h"

class demultiplexer_ip_link: public Simulator { 
  message::queue<link::Control> _sent_control_inputs;
  unsigned int _max_interface;
  Event _output;

public:
  demultiplexer_ip_link(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif