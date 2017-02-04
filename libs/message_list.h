#if !defined message_list_h
#define message_list_h

#include <queue>
#include <utility>
#include "simulator.h"

template<typename MSG>
class message_list{
  MSG current_msg;
  std::queue<std::pair<MSG,int>> queued_msgs; // pair (Message,port) 
	
public:
  message_list() {}
	
  void push(const MSG& m, int port) {
  	queued_msgs.push(std::make_pair(m,port));
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

#endif