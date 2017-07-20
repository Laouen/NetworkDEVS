//CPP:networkDEVS/demux_dn_int.cpp
#if !defined demux_dn_int_h
#define demux_dn_int_h

#include "template/dual_demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demux_dn_int demux_dn_int.h
 * 
 * @brief A specialization of the [DDemultiplexer](@ref DDemultiplexer) 
 * template model to demultiplex message::Multiplexed<dns::DomainName>, and
 * message::Multiplexed<int> messages.
 *  
 */

class demux_dn_int: public DDemultiplexer<dns::DomainName, int> {

public:
  demux_dn_int(const char *n): DDemultiplexer(n) {};
};

#endif