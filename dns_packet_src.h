//CPP:networkDEVS/dns_packet_src.cpp
#if !defined dns_packet_src_h
#define dns_packet_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class dns_packet_src dns_packet_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type dns::Packet from a file.
 *  
 */

class dns_packet_src: public input_stream<dns::Packet> {

public:
  dns_packet_src(const char *n): input_stream(n) {};
};

#endif