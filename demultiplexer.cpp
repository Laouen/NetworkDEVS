#include "demultiplexer.h"

void demultiplexer::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  _max_id = (unsigned int)va_arg(parameters,double);
  _output = Event(0,2*(_max_id+1));
}

double demultiplexer::ta(double t) {
  if (_output.port > _max_id) {
    return std::numeric_limits<double>::max();
  } else {
    return 0;
  }
}

void demultiplexer::dint(double t) {
  _output = Event(0,2*(_max_id+1));
}

void demultiplexer::dext(Event x, double t) {
  udp::Control control_to_deliver;
  udp::Multiplexed_packet packet_to_deliver;

  switch(x.port) {
  case 0:
    packet_to_deliver = *(udp::Multiplexed_packet*)x.value;
    if (packet_to_deliver.app_id <= _max_id) {
      _output = Event(_sent_packet_inputs.push(packet_to_deliver.packet,t),(2*packet_to_deliver.app_id));
    }
    break;
  case 1:
    control_to_deliver = *(udp::Control*)x.value;
    if (control_to_deliver.app_id <= _max_id) {
      _output = Event(_sent_control_inputs.push(control_to_deliver,t),(2*control_to_deliver.app_id)+1);
    }
    break;
  default:
    break;
  }
}

Event demultiplexer::lambda(double t) {
  return _output;
}

void demultiplexer::exit() {}