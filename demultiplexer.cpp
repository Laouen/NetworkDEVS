#include "demultiplexer.h"

void demultiplexer::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  _max_id = (uint)va_arg(parameters,double);
  _output = Event(0,_max_id+1);
}

double demultiplexer::ta(double t) {
  if (_output.port > _max_id) {
    return std::numeric_limits<double>::max();
  } else {
    return 0;
  }
}

void demultiplexer::dint(double t) {
  _output = Event(0,_max_id+1);
}

void demultiplexer::dext(Event x, double t) {
  udp::Control to_deliver = *(udp::Control*)x.value;
  if (to_deliver.app_id <= _max_id) {
    _output = Event(_sent_inputs.push(to_deliver,t),to_deliver.app_id);
  }
}

Event demultiplexer::lambda(double t) {
  return _output;
}

void demultiplexer::exit() {}