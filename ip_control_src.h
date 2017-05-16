//CPP:networkDEVS/ip_control_src.cpp
#if !defined ip_control_src_h
#define ip_control_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class ip_control_src ip_control_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type ip::Control from a file.
 *  
 */

class ip_control_src: public input_stream<ip::Control> {

public:
  ip_control_src(const char *n): input_stream(n) {};
};

#endif