#include "demultiplexer_g.h"

void demultiplexer_g::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  _max_interface = (unsigned int)va_arg(parameters,double);
  _output = Event(0,_max_interface+2);
}

double demultiplexer_g::ta(double t) {
  if (_output.port >= _max_interface) {
    return std::numeric_limits<double>::max();
  } else {
    return 0;
  }
}

void demultiplexer_g::dint(double t) {
  _output = Event(0,_max_interface+2);
}

void demultiplexer_g::dext(Event x, double t) {

  link::Control control_to_deliver = *(link::Control*)x.value;
  if (control_to_deliver.interface <= _max_interface) {
    _output = _sent_control_inputs.send(control_to_deliver, control_to_deliver.interface);
  }
}

Event demultiplexer_g::lambda(double t) {
  return _output;
}

void demultiplexer_g::exit() {}