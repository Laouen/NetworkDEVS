//CPP:networkDEVS/udp_control_src.cpp
#if !defined udp_control_src_h
#define udp_control_src_h

#include "template/input_stream.h"

class udp_control_src: public input_stream<udp::Control> {

public:
  udp_control_src(const char *n): input_stream(n) {};
};

#endif