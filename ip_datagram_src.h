//CPP:networkDEVS/ip_datagram_src.cpp
#if !defined ip_datagram_src_h
#define ip_datagram_src_h

#include "template/input_stream.h"

class ip_datagram_src: public input_stream<ip::Datagram> {

public:
  ip_datagram_src(const char *n): input_stream(n) {};
};

#endif