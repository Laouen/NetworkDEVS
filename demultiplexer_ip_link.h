//CPP:networkDEVS/demultiplexer_ip_link.cpp
#if !defined demultiplexer_ip_link_h
#define demultiplexer_ip_link_h

#include "template/demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demultiplexer_ip_link demultiplexer_ip_link.h
 * 
 * @brief A specialization of the [demultiplexer](@ref demultiplexer) 
 * template model to demultiplex message::Multiplexed<link::Control> messages.
 *  
 */

class demultiplexer_ip_link: public demultiplexer<link::Control> {

public:
  demultiplexer_ip_link(const char *n): demultiplexer(n) {};
};

#endif