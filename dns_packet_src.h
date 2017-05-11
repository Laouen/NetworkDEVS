//CPP:networkDEVS/dns_packet_src.cpp
#if !defined dns_packet_src_h
#define dns_packet_src_h

#include "template/input_stream.h"

class dns_packet_src: public input_stream<dns::Packet> {

public:
  dns_packet_src(const char *n): input_stream(n) {};
};

#endif