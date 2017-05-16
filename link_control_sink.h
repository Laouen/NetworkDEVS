//CPP:networkDEVS/link_control_sink.cpp
#if !defined link_control_sink_h
#define link_control_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class link_control_sink link_control_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type link::Control.
 *  
 */

class link_control_sink: public output_stream<link::Control> {

public:
  link_control_sink(const char *n): output_stream(n) {};
};

#endif