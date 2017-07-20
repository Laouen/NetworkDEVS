#if !defined message_queueh
#define message_queueh

#include <queue>
#include <utility>
#include "simulator.h"

/**
 * @namespace message
 */
namespace message {

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct message::Multiplexed message_queue.h
   * 
   * @brief Encapsualtes a message of type MSG and a interface number together.
   * @details This data type is used by the multiplexer model. The model expect 
   * as messages of type message::Multiplexed and it uses the interface number 
   * to select the correct output port where to send the message that comes in 
   * the Multiplexed instance. 
   * 
   * @tparam MSG Data type of the message to be carry by the multiplexed instance.
   */
  template<typename MSG>
  struct Multiplexed {
    MSG message;
    ushort interface;

    Multiplexed() {}
    Multiplexed(MSG other_message, ushort other_interface) {
      message = other_message;
      interface = other_interface;
    }

    Multiplexed(const Multiplexed<MSG>& other) {
      message = other.message;
      interface = other.interface;
    }
  };

  template<typename MSG>
  struct Message {
    MSG message;
    ushort port;
    ushort interface;

    Message() {}
    Message(const MSG& other_message, ushort other_interface, ushort other_port) {
      message = other_message;
      port = other_port;
      interface = other_interface;
    }
    Message(const Message<MSG>& other) {
      message = other.message;
      port = other.port;
      interface = other.interface;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct message::queue message_queue.h
   * 
   * @brief Implement a queue to output messages through ports in PowerDEVS models.
   * @details Each instance of a queue is asociated with a port and interface number 
   * and every message that is pushed into the queue will be sent through that port if 
   * no output port is explicitated in the push method.
   * If the multiplexed attribute is true, then the message is send as multiplexed using
   * the interface number.
   * This queue handles the void * requested by the PowerDEVS keeping the pointed
   * object alive until the next pop is done and the message was already 
   * delivered to the corresponding model or models. 
   * message::queue allows to push multiple messages to be send and send them 
   * one by one using the pop method that returns a PowerDEVS valid Event instance 
   * ready to be sent in the lambda function with the next message in the queue 
   * and the port set in it. 
   * 
   * @tparam MSG Data type of the messages that will output the queue
   */
  template<typename MSG>
  class queue{
    bool multiplexed;
    ushort default_port;
    ushort default_interface;
    MSG current_msg;
    Multiplexed<MSG> current_multiplexed_msg;
    std::queue<Message<MSG>> queued_msgs; 
    
  public:

    /**
     * @brief Default constructor that does no specifies neither port nor interface numbers.
     * @details defaul values are: 
     * port = 0. 
     * interface = 0;
     * multiplexed = true.
     */
    queue() {
      multiplexed = true;
      default_port = 0;
      default_interface = 0;
      this->clear();
    }

    /**
     * @brief Constructor with port number
     * @details This constructor initialize a new message::queue instance with
     * a default port number that will be used each time a message is pushed 
     * without passing a port number as parameter. 
     * Defaul values are:
     * interface = 0;
     * multiplexed = true.
     * 
     * @param other_default_port A unsigned short that specifies the default 
     * port number.
     */
    queue(ushort other_default_port) {
      multiplexed = true;
      default_interface = 0;
      default_port = other_default_port;
      this->clear();
    }

    /**
     * @brief Constructor with port and interface number
     * @details This constructor initialize a new message::queue instance with
     * default port and interface numbers that will be used each time a message 
     * is pushed without passing any port and interface numbers as parameter.
     * Defaul values are:
     * multiplexed = true.
     * 
     * @param other_default_port A unsigned short that specifies the default port number.
     * @param other_default_interface A unsigned short that specifies the default interface number. 
     */
    queue(ushort other_default_port, ushort other_default_interface) {
      multiplexed = true;
      default_port = other_default_port;
      default_interface = other_default_interface;
      this->clear();
    }

    /**
    * @brief removes all the pushed messages from the queue.
    */
    void clear() {
      while(!queued_msgs.empty()) {
        queued_msgs.pop();
      }
    }

    /**
    * @brief Interface to set a new value for the multiplexed attribute used to 
    * determines if messages must be send multiplexed or not.
    *
    * @param other_multiplexer A boolean that will be set as the new multiplexed
    * attribute value.
    */
    void set_multiplexed(bool other_multiplexer) {
      multiplexed = other_multiplexer;
    }
  	
    /**
     * @brief Adds a new message at the end of the queue seting a specific 
     * interface and port numbers where to send the message when poped.
     * 
     * @param message A MSG message to push.
     * @param interface An unsigned short that specifies the interface number where 
     * to multiplex the message when poped.
     * @param port An unsigned short that specifies the interfaz number where to send 
     * the message when poped.
     */
    void push(const MSG& message, ushort interface, ushort port) {
      queued_msgs.push(Message<MSG>(message, interface, port));
    };

    /**
     * @brief Adds a new message at the end of the queue seting a specific interface
     * and uses the default port to send the message when poped.
     * 
     * @param message A MSG message to push.
     * @param interface An unsigned short that specifies the interfaz number where
     * to multiplex the message when poped.
     */
    void push(const MSG& message, ushort interface) {
      queued_msgs.push(Message<MSG>(message, interface, default_port));
    };

    /**
     * @brief Adds a new message at the end of the queue seting the default interface
     * and the default port to send the message when poped.
     * 
     * @param message A MSG message to push.
     */
    void push(const MSG& message) {
      queued_msgs.push(Message<MSG>(message, default_interface, default_port));
    };

    /**
     * @brief Removes the next element from the queue in FIFO order and returns 
     * the PowerDEVS Event instance to send the message.
     * 
     * @details The removed object is stored in memory until the next pop is made 
     * to ensure the void * used by the simulator does point to a valid memory place.
     * 
     * @return A PowerDEVS Event instance that contains the poped message 
     * asociated with the port used to push the message in the queue.
     */
    Event pop() {
      Message<MSG> poped = queued_msgs.front();
      Event output;

      if (multiplexed) {
        current_multiplexed_msg = Multiplexed<MSG>(poped.message, poped.interface);
        output = Event(&current_multiplexed_msg, poped.port);
      } else {
        current_msg = poped.message;
        output = Event(&current_msg, poped.port);
      }

      queued_msgs.pop();
      return output;
    }


    /**
     * @brief This method does not push the message in the queue and instead it
     * directly generates a PowerDEVS Event instance with a pointer to the message
     * asociated with the port number passed as parameter and the and the default_interface.
     * 
     * @details This metod behave as a pop where the poped message is the one 
     * passed as parameter, the message pointer is handled in the same way a 
     * poped message pointer is handled. 
     * This method can be used to avoid the message be pushed if the one is 
     * a priority over all pushed mesasges.
     * 
     * @param message A MSG to be send.
     * @param port A unsigned short that indicates the port where to send the message.
     * @return A PowerDEVS Event instance that contains the message asociated 
     * with the port number passed as parameter.
     */
    Event send(const MSG& message, ushort port) {
      Event output;

      if (multiplexed) {
        current_multiplexed_msg = Multiplexed<MSG>(message, default_interface);
        output = Event(&current_multiplexed_msg, port);
      } else {
        current_msg = message;
        output = Event(&current_msg, port);
      }

      return output;
    }


    /**
     * @brief This method does not push the message in the queue and instead it
     * directly generates a PowerDEVS Event instance with a pointer to the message
     * asociated with the port and interface numbers passed as parameter.
     * 
     * @details This metod behave as a pop where the poped message is the one 
     * passed as parameter, the message pointer is handled in the same way a 
     * poped message pointer is handled. 
     * This method can be used to avoid the message be pushed if the one is 
     * a priority over all pushed mesasges.
     * 
     * @param message A MSG to be send.
     * @param interface A unsigned short that indicates the interface to multiplex the message.
     * @param port A unsigned short that indicates the port where to send the message.
     * @return A PowerDEVS Event instance that contains the message asociated 
     * with the port number passed as parameter.
     */
    Event send(const MSG& message, ushort interface, ushort port) {
      Event output;

      if (multiplexed) {
        current_multiplexed_msg = Multiplexed<MSG>(message, interface);
        output = Event(&current_multiplexed_msg, port);
      } else {
        current_msg = message;
        output = Event(&current_msg, port);
      }

      return output;
    }

    /**
     * @brief Checks whether the message::queue is empty or not.
     * @return True if the message:queue is empty, False otherwise.
     */
    bool empty() {
      return queued_msgs.empty();
    }
  };
}

#endif