//CPP:networkDEVS/udp_control_src.cpp
#if !defined udp_control_src_h
#define udp_control_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class udp_control_src udp_control_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type udp::Control from a file.
 *  
 */

class udp_control_src: public input_stream<udp::Control> {

public:
  udp_control_src(const char *n): input_stream(n) {};
};

#endif