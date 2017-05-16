//CPP:networkDEVS/link_frame_sink.cpp
#if !defined link_frame_sink_h
#define link_frame_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class link_frame_sink link_frame_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type link::Frame.
 *  
 */

class link_frame_sink: public output_stream<link::Frame> {

public:
  link_frame_sink(const char *n): output_stream(n) {};
};

#endif