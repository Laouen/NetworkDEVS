//CPP:networkDEVS/demux_frame_int.cpp
#if !defined demux_frame_int_h
#define demux_frame_int_h

#include "template/dual_demultiplexer.h"

/**
 * 
 * @author Lauen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demux_frame_int demux_frame_int.h
 * 
 * @brief A specialization of the [DDemultiplexer](@ref DDemultiplexer) 
 * template model to demultiplex message::Multiplexed<link::Frame>, and
 * message::Multiplexed<int> messages.
 *  
 */

class demux_frame_int: public DDemultiplexer<link::Frame, int> {

public:
  demux_frame_int(const char *n): DDemultiplexer(n) {};
};

#endif