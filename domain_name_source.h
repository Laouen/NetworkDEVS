//CPP:networkDEVS/domain_name_source.cpp
#if !defined domain_name_source_h
#define domain_name_source_h

#include "template/input_stream.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class domain_name_source domain_name_source.h
 * 
 * @brief A specialization of [input_stream](@ref input_stream) 
 * template model to generate input of type dns::DomainName from a file.
 *  
 */

class domain_name_source: public input_stream<dns::DomainName> {

public:
  domain_name_source(const char *n): input_stream(n) {};
};

#endif