//CPP:networkDEVS/link_frame_src.cpp
#if !defined link_frame_src_h
#define link_frame_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class link_frame_src link_frame_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type link::Frame from a file.
 *  
 */

class link_frame_src: public input_stream<link::Frame> {

public:
  link_frame_src(const char *n): input_stream(n) {};
};

#endif