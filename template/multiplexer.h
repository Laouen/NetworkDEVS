#if !defined multiplexer_h
#define multiplexer_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "../libs/message_queue.h"

#include "../structures/abstract_types.h"
#include "../structures/link.h"

template<typename MSG>
class multiplexer: public Simulator {
  Logger logger;

  message::queue<message::Multiplexed<MSG>> _output_queue;
  Event _output;

  double infinity = std::numeric_limits<double>::max();

public:
  multiplexer(const char *n): Simulator(n) {}

  void init(double t, ...) {
    // PowerDEVS parameters
    va_list parameters;
    va_start(parameters,t);

    // Set logger module name
    std::string module_name = va_arg(parameters,char*);
    logger.setModuleName("Multiplexer " + module_name);

    logger.info("Multiplexer initialized.");

    _output = Event(0,1);
  }

  double ta(double t) {
    return (_output.port > 0) ? infinity : 0;
  }

  void dint(double t) {
    _output = Event(0,1);
  }

  void dext(Event x, double t) {

    message::Multiplexed<MSG> m;
    m.message = *(MSG*)x.value;
    m.interface = x.port;
    _output = _output_queue.send(m, 0);
  }

  Event lambda(double t) {
    return _output;
  }

  void exit() {}
};

#endif