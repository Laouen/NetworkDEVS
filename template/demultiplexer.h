#if !defined demultiplexer_h
#define demultiplexer_h

#include <limits>

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include "../libs/message_queue.h"
#include "../libs/logger.h"

#include "../structures/abstract_types.h"
#include "../structures/link.h"

template<typename MSG>
class demultiplexer: public Simulator { 
  Logger logger;

  message::queue<MSG> _output_queue;
  unsigned int _max_interface;
  Event _output;

  double infinity = std::numeric_limits<double>::max();

public:

  demultiplexer(const char *n): Simulator(n) {}

  void init(double t, ...) {
    // PowerDEVS parameters
    va_list parameters;
    va_start(parameters,t);

    // Set logger module name
    std::string module_name = va_arg(parameters,char*);
    logger.setModuleName("Demultiplexer " + module_name);

    // Set interface amount
    _max_interface = (unsigned int)va_arg(parameters,double);
    _output = Event(0,_max_interface+2);

    logger.info("Initialized with " + 
                std::to_string(_max_interface) + 
                " interfaces");
  }

  double ta(double t) {
    return (_output.port >= _max_interface) ? infinity : 0;
  }

  void dint(double t) {
    _output = Event(0,_max_interface+2);
  }

  void dext(Event x, double t) {

    message::Multiplexed<MSG> multiplexed = *(message::Multiplexed<MSG>*)x.value;
    if (multiplexed.interface <= _max_interface) {
      _output = _output_queue.send(multiplexed.message, multiplexed.interface);
    }
  }

  Event lambda(double t) {
    return _output;
  }

  void exit() {}
};

#endif