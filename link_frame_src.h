//CPP:networkDEVS/link_frame_src.cpp
#if !defined link_frame_src_h
#define link_frame_src_h

#include "template/input_stream.h"

class link_frame_src: public input_stream<link::Frame> {

public:
  link_frame_src(const char *n): input_stream(n) {};
};

#endif