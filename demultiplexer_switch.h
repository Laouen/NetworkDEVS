//CPP:networkDEVS/demultiplexer_switch.cpp
#if !defined demultiplexer_switch_h
#define demultiplexer_switch_h

#include "template/demultiplexer.h"

class demultiplexer_switch: public demultiplexer<link::Frame> {

public:
	demultiplexer_switch(const char *n): demultiplexer(n) {};
};

#endif