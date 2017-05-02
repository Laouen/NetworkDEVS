//CPP:networkDEVS/demultiplexer_app_udp.cpp
#if !defined demultiplexer_app_udp_h
#define demultiplexer_app_udp_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "libs/message_queue.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/dns.h"
#include "structures/ip.h"
#include "structures/link.h"

class demultiplexer_app_udp: public Simulator { 
  message::queue<udp::Control> _sent_control_inputs;
  message::queue<dns::Packet> _sent_packet_inputs;
  unsigned int _max_id;
  Event _output;

public:
  demultiplexer_app_udp(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif