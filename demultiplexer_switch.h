//CPP:networkDEVS/demultiplexer_switch.cpp
#if !defined demultiplexer_switch_h
#define demultiplexer_switch_h

#include "template/demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demultiplexer_switch demultiplexer_switch.h
 * 
 * @brief A specialization of the [demultiplexer](@ref demultiplexer) 
 * template model to demultiplex message::Multiplexed<link::Frame> messages.
 *  
 */

class demultiplexer_switch: public demultiplexer<link::Frame> {

public:
	demultiplexer_switch(const char *n): demultiplexer(n) {};
};

#endif