#if !defined layer_h
#define layer_h

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

// basic std types
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <utility>
#include <iostream>
#include <limits>
#include <iomanip>

#include "libs/message_list.h"
#include "libs/parser.h"
#include "libs/logger.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/ipv4.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"
#include "structures/link.h"
#include "structures/swp.h"

/**
 * Basic layer model to inheritate 
 */
template <typename DH, typename CH, typename DL, typename CL>
class Layer: public Simulator { 

protected:
  // Logger
  Logger logger;

  // comunication queues
  std::queue<CH> higher_layer_ctrl_in;
  std::queue<CL> lower_layer_ctrl_in;
  message_list<CH> higher_layer_ctrl_out;
  message_list<CL> lower_layer_ctrl_out;
  // data queues
  std::queue<DH> higher_layer_data_in;
  std::queue<DL> lower_layer_data_in;
  message_list<DH> higher_layer_data_out;
  message_list<DL> lower_layer_data_out;

  Event output;
  std::queue<Event> outputs;

  double next_internal;
  double last_transition;

  /********** TIMES ***************/
  double infinity = std::numeric_limits<double>::max();
  double send_time = 0.001;

  /********* Private methods *********/
  bool queuedMsgs() const {
    return  !higher_layer_ctrl_in.empty() ||
            !lower_layer_ctrl_in.empty() ||
            !higher_layer_data_in.empty() ||
            !lower_layer_data_in.empty();
  }

public:

  Layer(const char *n): Simulator(n) {};

  void dint(double t) {

    output = Event(0,5);
    next_internal = infinity;
    
    // this may cause some starvation if there is too many outputs
    if (!outputs.empty()) {
      output = outputs.front();
      outputs.pop();
      next_internal = send_time;
      last_transition = t;
      return;
    }

    this->dinternal(t);
    last_transition = t;
  }
  
  void dext(Event x, double t) {
    switch(x.port) {
    case 0:
      higher_layer_data_in.push(*(DH*)x.value);
      break;
    case 1:
      higher_layer_ctrl_in.push(*(CH*)x.value);
      break;
    case 2:
      lower_layer_data_in.push(*(DL*)x.value);
      break;
    case 3:
      lower_layer_ctrl_in.push(*(CL*)x.value);
      break;
    default:
      logger.error("Invalid port.");
      break;
    }

    if (next_internal < infinity)
      next_internal -= (t-last_transition);
    else if (this->queuedMsgs()) 
      next_internal = 0;

    this->dexternal(t);
    last_transition = t;
  }

  virtual void dinternal(double t) {}
  virtual void dexternal(double t) {}
};

#endif