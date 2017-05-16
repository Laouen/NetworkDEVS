//CPP:networkDEVS/udp_control_sink.cpp
#if !defined udp_control_sink_h
#define udp_control_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class udp_control_sink udp_control_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type udp::Control.
 *  
 */

class udp_control_sink: public output_stream<udp::Control> {

public:
  udp_control_sink(const char *n): output_stream(n) {};
};

#endif