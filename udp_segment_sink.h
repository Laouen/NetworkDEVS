//CPP:networkDEVS/udp_segment_sink.cpp
#if !defined udp_segment_sink_h
#define udp_segment_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class udp_segment_sink udp_segment_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type udp::Segmento.
 *  
 */

class udp_segment_sink: public output_stream<udp::Segment> {

public:
  udp_segment_sink(const char *n): output_stream(n) {};
};

#endif