//CPP:networkDEVS/ip_datagram_sink.cpp
#if !defined ip_datagram_sink_h
#define ip_datagram_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class ip_datagram_sink ip_datagram_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type ip::Datagram.
 *  
 */

class ip_datagram_sink: public output_stream<ip::Datagram> {

public:
  ip_datagram_sink(const char *n): output_stream(n) {};
};

#endif