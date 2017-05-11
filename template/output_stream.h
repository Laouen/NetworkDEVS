#if !defined output_stream_h
#define output_stream_h

#include "layer.h"

template<typename DATA>
class output_stream: public Simulator {
  Logger logger;
  std::ofstream file;
  double infinity = std::numeric_limits<double>::max();

public:
  output_stream(const char *n): Simulator(n) {};

  void init(double t, ...) {
    // PowerDEVS parameters
    va_list parameters;
    va_start(parameters,t);

    // Set logger module name
    std::string module_name = va_arg(parameters,char*);
    logger.setModuleName("Input stream " + module_name);

    const char* file_path = va_arg(parameters,char*);
    file.open(file_path);
  }

  double ta(double t) {
    return infinity;
  }

  void dint(double t) {}

  void dext(Event x, double t) {

    if (x.port == 0) {
      DATA m = *(DATA*)x.value;
      file << "time:" << t << std::endl;
      file << m << std::endl << std::endl;
    }
  }

  Event lambda(double t) {
    return Event(0,0);
  }

  void exit() {
    file.close();
  }
};

#endif