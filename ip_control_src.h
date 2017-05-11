//CPP:networkDEVS/ip_control_src.cpp
#if !defined ip_control_src_h
#define ip_control_src_h

#include "template/input_stream.h"

class ip_control_src: public input_stream<ip::Control> {

public:
  ip_control_src(const char *n): input_stream(n) {};
};

#endif