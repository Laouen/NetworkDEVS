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

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct message::queue message_queue.h
   * 
   * @brief Implement a queue to output messages through ports in PowerDEVS models.
   * @details Each instance of a queue is asociated with a port number and every 
   * message that is pushed into the queue will be sent through that port if 
   * no output port is explicitated in the push method.
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
    ushort default_port;
    ushort default_interface;
    Multiplexed<MSG> current_msg;
    std::queue<std::pair<Multiplexed<MSG>,int>> queued_msgs; // pair (Multiplexed<Message>,port) 
    
  public:

    /**
     * @brief Default constructor that does no specifies neither port nor interface numbers.
     * @details If this constructor is used and the port and interface nubers are never
     * specified, each push must be done passing the port and interface as parameters, 
     * otherwise the port and interface numbers are zero.
     */
    queue() {
      default_port = 0;
      default_interface = 0;
      queued_msgs.clear();
    }

    /**
     * @brief Constructor with port number
     * @details This constructor initialize a new message::queue instance with
     * a default port number that will be used each time a message is pushed 
     * without passing a port number as parameter, but it does not specifies a
     * interface number which will be set in zero.
     * 
     * @param other_default_port A unsigned short that specifies the default 
     * port number.
     */
    queue(ushort other_default_port) {
      default_port = other_default_port;
      default_interface = 0;
    }

    /**
     * @brief Constructor with port and interface number
     * @details This constructor initialize a new message::queue instance with
     * default port and interface numbers that will be used each time a message 
     * is pushed without passing any port and interface numbers as parameter.
     * 
     * @param other_default_port A unsigned short that specifies the default port number.
     * @param other_default_interface A unsigned short that specifies the default interface number. 
     */
    queue(ushort other_default_port, ushort other_default_interface) {
      default_port = other_default_port;
      default_interface = other_default_interface;
    }
  	
    /**
     * @brief Adds a new message at the end of the queue seting a specific 
     * interface and port numbers where to send the message when poped.
     * 
     * @param m A MSG message to push.
     * @param port An unsigned short that specifies the port number where to send 
     * the message when poped.
     * @param port An unsigned short that specifies the interfaz number where to send 
     * the message when poped.
     */
    void push(const MSG& message, ushort interface, ushort port) {
      queued_msgs.push(std::make_pair(Multiplexed<MSG>(message, interface), port));
    };

    /**
     * @brief Adds a new message at the end of the queue seting a specific interface
     * and uses the default port to send the message when poped.
     * 
     * @param m A MSG message to push.
     * @param port An unsigned short that specifies the interfaz number where to send 
     * the message when poped.
     */
    void push(const MSG& message, ushort interface) {
      queued_msgs.push(std::make_pair(Multiplexed<MSG>(message, interface), default_port));
    };

    /**
     * @brief Adds a new message at the end of the queue seting the default interface
     * and the default port to send the message when poped.
     * 
     * @param m A MSG message to push.
     */
    void push(const MSG& message) {
      queued_msgs.push(std::make_pair(Multiplexed<MSG>(message, default_interface), default_port));
    };

    /**
     * @brief Removes the next element from the queue in FIFO order and returns 
     * the PowerDEVS Event instance to send the message from one model to another.
     * 
     * @details The removed object is stored in memory until the next pop is made 
     * to ensure the void * used by the simulator does point to a valid memory place.
     * 
     * @return A PowerDEVS Event instance that contains the poped message 
     * asociated with the port used to push the message in the queue.
     */
    Event pop() {
      current_msg = queued_msgs.front().first;
      ushort port = queued_msgs.front().second;
      queued_msgs.pop();
      return Event(&current_msg, port);
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
     * @return A PowerDEVS Event instance that contains the message asociated 
     * with the port number passed as parameter.
     */
    Event send(const MSG& message, ushort interface, ushort port) {
      current_msg = Multiplexed<MSG>(message, interface);
      return Event(&current_msg, port);
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