//CPP:networkDEVS/output_stream.cpp
#if !defined output_stream_h
#define output_stream_h

#include <limits>
#include <string>
#include <fstream>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"
#include "structures/link.h"

class output_stream: public Simulator {
  std::ofstream file;

public:
  output_stream(const char *n): Simulator(n) {};
  void init(double, ...);
  double ta(double t);
  void dint(double);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif