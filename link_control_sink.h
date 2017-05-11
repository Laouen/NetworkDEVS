//CPP:networkDEVS/link_control_sink.cpp
#if !defined link_control_sink_h
#define link_control_sink_h

#include "template/output_stream.h"

class link_control_sink: public output_stream<link::Control, int> {

public:
  link_control_sink(const char *n): output_stream(n) {};
};

#endif