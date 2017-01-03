#include "input_stream.h"

void input_stream::init(double t,...) {
  // PowerDEVS parameters

  va_list parameters;
  va_start(parameters,t);
  const char* file_path = va_arg(parameters,char*);

  _parser.open_file(file_path);
  dint(0.0);
}

double input_stream::ta(double t) {
  printLog("[input stream] ta ");
  return _next_time;
}

void input_stream::dint(double t) {
  printLog("[input stream] dint ");
  try {
    std::pair<double,app::Control> parsed_line = _parser.next_input();
    _next_time = parsed_line.first - t;

    if (_next_time < 0.0) {
      throw std::exception();
    }
    _next_input = parsed_line.second;

  } catch(std::exception& e) {
    _next_time = std::numeric_limits<double>::max();
    _next_input = app::Control();
  }
}

void input_stream::dext(Event x, double t) {
  printLog("[input stream] dext ");
  throw std::exception();
}

Event input_stream::lambda(double t) {
  printLog("[input stream] lambda ");
  return Event(_sent_inputs.push(_next_input,t),0);
}

void input_stream::exit() {}