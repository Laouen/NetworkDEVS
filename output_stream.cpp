#include "output_stream.h"

void output_stream::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);
  const char* file_path = va_arg(parameters,char*);
  file.open(file_path);
}

double output_stream::ta(double t) {
  return std::numeric_limits<double>::max();
}

void output_stream::dint(double t) {}

void output_stream::dext(Event x, double t) {

  if (x.port == 0) {
    app::Packet m = *(app::Packet*)x.value;
    file << "time:" << t << std::endl;
    file << "App::Packet: " << m << std::endl << std::endl;
  } else if (x.port == 1) {
    udp::Control m = *(udp::Control*)x.value;
    file << "time:" << t << std::endl;
    file << m << std::endl;
  }
}

Event output_stream::lambda(double t) {
  return Event(0,0);
}

void output_stream::exit() {
  file.close();
}