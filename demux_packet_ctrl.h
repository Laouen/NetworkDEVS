//CPP:networkDEVS/demux_packet_ctrl.cpp
#if !defined demux_packet_ctrl_h
#define demux_packet_ctrl_h

#include "template/dual_demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demux_packet_ctrl demux_packet_ctrl.h
 * 
 * @brief A specialization of the [DDemultiplexer](@ref DDemultiplexer) 
 * template model to demultiplex message::Multiplexed<dns::Packet>, and
 * message::Multiplexed<udp::Control> messages.
 *  
 */

class demux_packet_ctrl: public DDemultiplexer<dns::Packet, udp::Control> {

public:
  demux_packet_ctrl(const char *n): DDemultiplexer(n) {};
};

#endif