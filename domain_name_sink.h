//CPP:networkDEVS/domain_name_sink.cpp
#if !defined domain_name_sink_h
#define domain_name_sink_h

#include "template/output_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class domain_name_sink domain_name_sink.h
 * 
 * @brief A specialization of [output_stream](@ref output_stream) 
 * template model to write in a file output messages of type dns::DomainName.
 *  
 */

class domain_name_sink: public output_stream<dns::DomainName> {

public:
  domain_name_sink(const char *n): output_stream(n) {};
};

#endif