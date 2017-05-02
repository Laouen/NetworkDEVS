#if !defined message_h
#define message_h

#include <queue>
#include <utility>
#include "simulator.h"

namespace message {

template<typename MSG>
class queue{
  ushort default_port;
  MSG current_msg;
  std::queue<std::pair<MSG,int>> queued_msgs; // pair (Message,port) 
  
public:
  queue() {}
  queue(ushort other_default_port) {
    default_port = other_default_port;
  }
	
  void push(const MSG& m, int port) {
  	queued_msgs.push(std::make_pair(m,port));
  };

  void push(const MSG& m) {
    queued_msgs.push(std::make_pair(m,default_port));
  };

  Event pop() {
    current_msg = queued_msgs.front().first;
    int port = queued_msgs.front().second;
    queued_msgs.pop();
    return Event(&current_msg, port);
  }

  Event send(const MSG& m, int port) {
    current_msg = m;
    return Event(&current_msg, port);
  }

  bool empty() {
    return queued_msgs.empty();
  }
};
}

#endif