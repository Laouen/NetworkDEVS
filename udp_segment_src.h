//CPP:networkDEVS/udp_segment_src.cpp
#if !defined udp_segment_src_h
#define udp_segment_src_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class udp_segment_src udp_segment_src.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type udp::Segment from a file.
 *  
 */

class udp_segment_src: public input_stream<udp::Segment> {

public:
  udp_segment_src(const char *n): input_stream(n) {};
};

#endif