//CPP:networkDEVS/demultiplexer_ip_link.cpp
#if !defined demultiplexer_ip_link_h
#define demultiplexer_ip_link_h

#include "template/demultiplexer.h"

class demultiplexer_ip_link: public demultiplexer<link::Control> {

public:
  demultiplexer_ip_link(const char *n): demultiplexer(n) {};
};

#endif