//CPP:networkDEVS/dns_packet_sink.cpp
#if !defined dns_packet_sink_h
#define dns_packet_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class dns_packet_sink dns_packet_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type dns::Packet.
 *  
 */

class dns_packet_sink: public output_stream<dns::Packet> {

public:
  dns_packet_sink(const char *n): output_stream(n) {};
};

#endif