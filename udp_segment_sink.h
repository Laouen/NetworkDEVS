//CPP:networkDEVS/udp_segment_sink.cpp
#if !defined udp_segment_sink_h
#define udp_segment_sink_h

#include "template/output_stream.h"

class udp_segment_sink: public output_stream<udp::Segment> {

public:
  udp_segment_sink(const char *n): output_stream(n) {};
};

#endif