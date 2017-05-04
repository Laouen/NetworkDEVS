#if !defined input_stream_h
#define input_stream_h

#include "layer.h"

template<typename INPUT>
class input_stream: public Simulator { 

  Logger logger;
  Parser<INPUT> _parser;
  double _next_time;
  message::queue<INPUT> _sent_inputs;
  INPUT _next_input;

public:
  input_stream(const char *n): Simulator(n) {};
  
  void init(double t, ...) {
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

  double ta(double t) {
    return _next_time;
  }

  void dint(double t) {
    try {
      std::pair<double,INPUT> parsed_line = _parser.next_timed_input();
      _next_time = parsed_line.first - t;
      _next_input = parsed_line.second;

      if (_next_time < 0.0) {
        throw std::exception();
      }
    } catch(std::exception& e) {
      _next_time = std::numeric_limits<double>::max();
      _next_input = INPUT();
    }
  }

  void dext(Event x, double t) {
    throw std::exception();
  }

  Event lambda(double t) {
    return _sent_inputs.send(_next_input,0);
  }

  void exit() {}
};

#endif