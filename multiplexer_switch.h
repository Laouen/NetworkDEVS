//CPP:networkDEVS/multiplexer_switch.cpp
#if !defined multiplexer_switch_h
#define multiplexer_switch_h

#include "template/multiplexer.h"

class multiplexer_switch: public multiplexer<link::Frame> {

public:
	multiplexer_switch(const char *n): multiplexer(n) {};
};

#endif