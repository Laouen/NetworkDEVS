#include "input_stream.h"

void input_stream::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters,char*);
  logger.setModuleName("Input stream " + module_name);

  const char* file_path = va_arg(parameters,char*);

  _parser.open_file(file_path);
  dint(0.0);
}

double input_stream::ta(double t) {
  // logger.debug("ta: " + std::to_string(_next_time));
  return _next_time;
}

void input_stream::dint(double t) {
  try {
    std::pair<double,dns::DomainName> parsed_line = _parser.next_timed_input();
    _next_time = parsed_line.first - t;
    _next_input = parsed_line.second;

    if (_next_time < 0.0) {
      throw std::exception();
    }
  } catch(std::exception& e) {
    _next_time = std::numeric_limits<double>::max();
    _next_input = dns::DomainName();
  }
}

void input_stream::dext(Event x, double t) {
  throw std::exception();
}

Event input_stream::lambda(double t) {
  // logger.debug("lambda");
  return _sent_inputs.send(_next_input,0);
}

void input_stream::exit() {}