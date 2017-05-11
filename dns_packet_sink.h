//CPP:networkDEVS/dns_packet_sink.cpp
#if !defined dns_packet_sink_h
#define dns_packet_sink_h

#include "template/output_stream.h"

class dns_packet_sink: public output_stream<dns::Packet> {

public:
  dns_packet_sink(const char *n): output_stream(n) {};
};

#endif