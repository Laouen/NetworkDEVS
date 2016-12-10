//CPP:networkDEVS/input_stream.cpp
#if !defined input_stream_h
#define input_stream_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "libs/message_list.h"
#include "libs/parser.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"

class input_stream: public Simulator { 

  Parser<app::Control> _parser;
  double _next_time;
  message_list<app::Control> _sent_inputs;
  app::Control _next_input;

public:
  input_stream(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif