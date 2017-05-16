//CPP:networkDEVS/link_control_src.cpp
#if !defined link_control_src_h
#define link_control_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class link_control_src link_control_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type link::Control from a file.
 *  
 */

class link_control_src: public input_stream<link::Control> {

public:
  link_control_src(const char *n): input_stream(n) {};
};

#endif