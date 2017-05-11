//CPP:networkDEVS/udp_segment_src.cpp
#if !defined udp_segment_src_h
#define udp_segment_src_h

#include "template/input_stream.h"

class udp_segment_src: public input_stream<udp::Segment> {

public:
  udp_segment_src(const char *n): input_stream(n) {};
};

#endif