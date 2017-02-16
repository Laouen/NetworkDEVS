#include "multiplexer_switch.h"

void multiplexer_switch::init(double t,...) {
  _output = Event(0,1);
}

double multiplexer_switch::ta(double t) {
  if (_output.port > 0) {
    return std::numeric_limits<double>::max(); // infinity
  } else {
    return 0;
  }
}

void multiplexer_switch::dint(double t) {
  _output = Event(0,1);
}

void multiplexer_switch::dext(Event x, double t) {

  link::Multiplexed_frame m;
  m.frame = *(link::Frame*)x.value;
  m.interface = x.port;
  _output = _sent_frame_out.send(m, 0);
}

Event multiplexer_switch::lambda(double t) {
  return _output;
}

void multiplexer_switch::exit() {}