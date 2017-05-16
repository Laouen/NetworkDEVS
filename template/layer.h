#if !defined layer_h
#define layer_h

#include <limits>

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

#include "../libs/message_queue.h"
#include "../libs/parser.h"
#include "../libs/logger.h"

#include "../structures/abstract_types.h"
#include "../structures/socket.h"
#include "../structures/ipv4.h"
#include "../structures/dns.h"
#include "../structures/udp.h"
#include "../structures/ip.h"
#include "../structures/link.h"
#include "../structures/swp.h"
#include "../structures/sw.h"

/**
 * 
 * @author Laouen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class Layer layer.h
 * 
 * @brief This class is a meta model that implements all the common behavior of 
 * a network devise layer from the OSI model and must be inherited to implement
 * the protocol of the layer.
 *  
 * @details This model has four input port and four output ports to comunicate
 * to the upper and lower layers of the OSI model using a different port for the
 * data (Packet, Segment, Datagram, Frame, etc.) and for the control messages
 * send between the layers due to the protocol they implement.
 * The mapping between the input/output ports of a layer and the data kind that 
 * should be send through them is specified as next:
 * 
 * Input queues:
 * * std::queue<DH> higher_layer_data_in; // Input Port 0
 * * std::queue<DL> lower_layer_data_in;  // Input Port 1
 * * std::queue<CH> higher_layer_ctrl_in; // Input Port 2 
 * * std::queue<CL> lower_layer_ctrl_in;  // Input Port 3
 * 
 * Output queues:
 * * message::queue<DH2> higher_layer_data_out = 0; // Output Port 0 
 * * message::queue<DL2> lower_layer_data_out = 1;  // Output Port 1
 * * message::queue<CH2> higher_layer_ctrl_out = 3; // Output Port 2
 * * message::queue<CL2> lower_layer_ctrl_out = 3;  // Output Port 3 
 * 
 * How the incoming messages are queued in the corresponding input queues and how
 * the pushed messages in the output queues are send through the correct output
 * ports is a task the protocol developer does not need to do because the 
 * it is implemented in the template model Layer.
 * 
 * The protocol developer only need to implement the dinternal method where the
 * protocol must be implemented and must uses the queue with the methods front, 
 * pop and push to read incoming messages from the input queues and to send 
 * messages through the output ports to other layers.
 * 
 * Note: modelers familirized with DEVS and PowerDEVS can use the dexternal method
 * to implement some behavior when external transition is occuring.
 * 
 * The processing flow is the following: 
 * 1. The model stays in IDLE until a message arrives. 
 * 2. When that happens, the dinternal method is used to process the message.
 * 3. When dinternal finish the model uses the value stored in next_internal to set 
 * how much virtual time (the simulated time) has take the protocol to process the 
 * dinternal. In the dinternal method this variable must be correctly set with a value 
 * grater than zero. 
 * 4. If there is messages in the output queues, the model iterate through all 
 * of them to send all of them. 
 * 5. If there is no messages in the input queues the model comes back to IDLE state.
 * 6. If it still are messages in the input queues, the model repeat the whole process.
 * 
 * Note: The messages must be poped from the input queues by the protocol in the diternal
 * method, if this does not happen, the model will trigger the dinternal method again
 * until they are all empty. 
 * 
 * @tparam DH The data type of the messages send trough output port 0
 * @tparam CH The data type of the messages send trough output port 1
 * @tparam DL The data type of the messages send trough output port 2
 * @tparam CL The data type of the messages send trough output port 3
 * @tparam DH2 The data type of the messages received from the input port 0 (by default is the same as DH)
 * @tparam CH2 The data type of the messages received from the input port 1 (by default is the same as CH)
 * @tparam DL2 The data type of the messages received from the input port 2 (by default is the same as DL)
 * @tparam CL2 The data type of the messages received from the input port 3 (by default is the same as CL)
 */
template <typename DH, typename CH, typename DL, typename CL, typename DH2 = DH, typename CH2 = CH, typename DL2 = DL, typename CL2 = CL>
class Layer: public Simulator { 

protected:
  // Logger
  Logger logger;

  // input queues
  std::queue<DH> higher_layer_data_in;
  std::queue<DL> lower_layer_data_in;
  std::queue<CH> higher_layer_ctrl_in;
  std::queue<CL> lower_layer_ctrl_in;
  
  // output queues
  message::queue<DH2> higher_layer_data_out = 0;
  message::queue<DL2> lower_layer_data_out = 2;
  message::queue<CH2> higher_layer_ctrl_out = 1;
  message::queue<CL2> lower_layer_ctrl_out = 3;

  Event output;

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
    
    if (!higher_layer_ctrl_out.empty()) {
      output = higher_layer_ctrl_out.pop();
      next_internal = send_time;
      last_transition = t;
      return;
    }

    if (!lower_layer_ctrl_out.empty()) {
      output = lower_layer_ctrl_out.pop();
      next_internal = send_time;
      last_transition = t;
      return;
    }

    if (!higher_layer_data_out.empty()) {
      output = higher_layer_data_out.pop();
      next_internal = send_time;
      last_transition = t;
      return;
    }

    if (!lower_layer_data_out.empty()) {
      output = lower_layer_data_out.pop();
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
      logger.error("[Layer.h] Invalid port.");
      break;
    }

    if (next_internal < infinity)
      next_internal -= (t-last_transition);
    else if (this->queuedMsgs()) 
      next_internal = 0;

    this->dexternal(t);
    last_transition = t;
  }

  /**
   * @brief This method is virtual and must be overloaded with the protocol.
   * @details This method is called while the input queues has messages to process
   * and the procotol should be implemented here..
   * 
   * @param t The virtual global time of the simulation at which the method is
   * triggered.
   */
  virtual void dinternal(double t) {}

  /**
   * @brief This method is virtual and can be overloaded with the protocol.
   * @details This method is called each time an external transition takes place.
   * 
   * @param t The virtual global time of the simulation at which the method is
   * triggered.
   */
  virtual void dexternal(double t) {}
};

#endif