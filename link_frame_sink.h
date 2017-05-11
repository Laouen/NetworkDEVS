//CPP:networkDEVS/link_frame_sink.cpp
#if !defined link_frame_sink_h
#define link_frame_sink_h

#include "template/output_stream.h"

class link_frame_sink: public output_stream<link::Frame> {

public:
  link_frame_sink(const char *n): output_stream(n) {};
};

#endif