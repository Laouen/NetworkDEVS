#include "demultiplexer_ip_link.h"

void demultiplexer_ip_link::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  _max_interface = (unsigned int)va_arg(parameters,double);
  _output = Event(0,_max_interface+2);
}

double demultiplexer_ip_link::ta(double t) {
  if (_output.port >= _max_interface) {
    return std::numeric_limits<double>::max();
  } else {
    return 0;
  }
}

void demultiplexer_ip_link::dint(double t) {
  _output = Event(0,_max_interface+2);
}

void demultiplexer_ip_link::dext(Event x, double t) {

  link::Control control_to_deliver = *(link::Control*)x.value;
  if (control_to_deliver.interface <= _max_interface) {
    _output = _sent_control_inputs.send(control_to_deliver, control_to_deliver.interface);
  }
}

Event demultiplexer_ip_link::lambda(double t) {
  return _output;
}

void demultiplexer_ip_link::exit() {}