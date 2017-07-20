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

/**
 * 
 * @author Laouen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class demultiplexer demultiplexer.h
 * 
 * @brief Template meta model of a demultiplexer that receives a 
 * message::multiplexed<MSG> in the input port number zero and returns 
 * the encapsulated message through the port indicated by the interface 
 * field of the multiplexed instance.
 * 
 * @details This model must be inherited by a new class with the template MSG
 * parameter specified with the same data type that is specified in the
 * message::multiplexed type that will arrives to this model.
 * 
 * The new class must follow the PowerDEVS specifications and a .cpp file must 
 * exist even if it is an empty file in order to correctly compile the model 
 * from the PowerDEVS IDE.
 * 
 * The next rules must be followed:
 * 1. This file must be included in the file where the new class is declared.
 * 2. The new class name must be all lower case.
 * 3. The new class must have in the first line the next comment: 
 *        //CPP:networkDEVS/new_class_name.cpp.
 * 4. The file networkDEVS/new_class_name.cpp must exist as an empty file.
 * 5. The constructor of the new class must be specified in the public section as shown here: 
 *        new_class_name(const char *n): demultiplexer(n) {};
 * 
 * The parameters to specifie in the PowerDEVS IDE (right click in the atomic
 * model -> edit -> parameters) are the nexts:
 * 
      name    |      type     |                    description
:-----------: | :-----------: | :--------------------------------------------------:
 module name  |    String     | a name used to tag the logs generated by this model
 * 
 * @tparam MSG The data type of the template parameter MSG of the 
 * message::Multiplexed message that arrives in the input port zero.
 */

template<typename MSG>
class demultiplexer: public Simulator {
  Logger logger;

  message::queue<MSG> _output_queue;
  int _max_interface;
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
    logger.setModuleName("Demux " + module_name);

    // Set interface amount
    _max_interface = (unsigned int)va_arg(parameters,double);
    _output = Event(0,_max_interface+2);

    logger.info("Initialized with " + 
                std::to_string(_max_interface) + 
                " interfaces");

    // send messages not multiplexed
    _output_queue.set_multiplexed(false);
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