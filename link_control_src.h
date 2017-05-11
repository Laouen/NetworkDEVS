//CPP:networkDEVS/link_control_src.cpp
#if !defined link_control_src_h
#define link_control_src_h

#include "template/input_stream.h"

class link_control_src: public input_stream<link::Control> {

public:
  link_control_src(const char *n): input_stream(n) {};
};

#endif