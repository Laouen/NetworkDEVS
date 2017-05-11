//CPP:networkDEVS/ip_datagram_sink.cpp
#if !defined ip_datagram_sink_h
#define ip_datagram_sink_h

#include "template/output_stream.h"

class ip_datagram_sink: public output_stream<ip::Datagram> {

public:
  ip_datagram_sink(const char *n): output_stream(n) {};
};

#endif