#include "demultiplexer_switch.h"

void demultiplexer_switch::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  _max_interface = (unsigned int)va_arg(parameters,double);
  _output = Event(0,_max_interface+2);
}

double demultiplexer_switch::ta(double t) {
  if (_output.port >= _max_interface) {
    return std::numeric_limits<double>::max();
  } else {
    return 0;
  }
}

void demultiplexer_switch::dint(double t) {
  _output = Event(0,_max_interface+2);
}

void demultiplexer_switch::dext(Event x, double t) {

  link::Multiplexed_frame frame_to_deliver = *(link::Multiplexed_frame*)x.value;
  if (frame_to_deliver.interface <= _max_interface) {
    _output = _sent_frame_out.send(frame_to_deliver.frame, frame_to_deliver.interface);
  }
}

Event demultiplexer_switch::lambda(double t) {
  return _output;
}

void demultiplexer_switch::exit() {}