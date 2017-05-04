//CPP:networkDEVS/domain_name_source.cpp
#if !defined domain_name_source_h
#define domain_name_source_h

#include "template/input_stream.h"

class domain_name_source: public input_stream<dns::DomainName> {

public:
  domain_name_source(const char *n): input_stream(n) {};
};

#endif