//CPP:networkDEVS/ip_datagram_src.cpp
#if !defined ip_datagram_src_h
#define ip_datagram_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class ip_datagram_src ip_datagram_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type ip::Datagram from a file.
 *  
 */

class ip_datagram_src: public input_stream<ip::Datagram> {

public:
  ip_datagram_src(const char *n): input_stream(n) {};
};

#endif