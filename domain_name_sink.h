//CPP:networkDEVS/domain_name_sink.cpp
#if !defined domain_name_sink_h
#define domain_name_sink_h

#include "template/output_stream.h"

class domain_name_sink: public output_stream<dns::DomainName> {

public:
  domain_name_sink(const char *n): output_stream(n) {};
};

#endif