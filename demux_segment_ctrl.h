//CPP:networkDEVS/demux_segment_ctrl.cpp
#if !defined demux_segment_ctrl_h
#define demux_segment_ctrl_h

#include "template/dual_demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demux_segment_ctrl demux_segment_ctrl.h
 * 
 * @brief A specialization of the [DDemultiplexer](@ref DDemultiplexer) 
 * template model to demultiplex message::Multiplexed<udp::Segment>, and
 * message::Multiplexed<ip::Control> messages.
 *  
 */

class demux_segment_ctrl: public DDemultiplexer<udp::Segment, ip::Control> {

public:
  demux_segment_ctrl(const char *n): DDemultiplexer(n) {};
};

#endif