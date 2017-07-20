//CPP:networkDEVS/demux_datagram_ctrl.cpp
#if !defined demux_datagram_ctrl_h
#define demux_datagram_ctrl_h

#include "template/dual_demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demux_datagram_ctrl demux_datagram_ctrl.h
 * 
 * @brief A specialization of the [DDemultiplexer](@ref DDemultiplexer) 
 * template model to demultiplex message::Multiplexed<ip::Datagram>, and
 * message::Multiplexed<link::Control> messages.
 *  
 */

class demux_datagram_ctrl: public DDemultiplexer<ip::Datagram, link::Control> {

public:
  demux_datagram_ctrl(const char *n): DDemultiplexer(n) {};
};

#endif