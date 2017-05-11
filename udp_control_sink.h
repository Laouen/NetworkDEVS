//CPP:networkDEVS/udp_control_sink.cpp
#if !defined udp_control_sink_h
#define udp_control_sink_h

#include "template/output_stream.h"

class udp_control_sink: public output_stream<udp::Control> {

public:
  udp_control_sink(const char *n): output_stream(n) {};
};

#endif