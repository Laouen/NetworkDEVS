#if !defined message_list_h
#define message_list_h

#include <list>
#include <utility>
#include <string>

#include "simulator.h"

template<typename T>
class message_list{
  std::list<std::pair<T,double>> msgs;
  double _last_time = -1.0;
	
public:
  message_list() {}
  ~message_list() { msgs.clear(); }
	
  T* push(const T& m, double t) {

    if (t < _last_time) {
      printLog("BadTime\n");
      printLog(std::to_string(t).c_str());
      printLog(" < ");
      printLog(std::to_string(_last_time).c_str());
      throw std::exception();
    }

    _last_time = t;
    remove_obsolete_messages();
    msgs.push_back(std::make_pair(m,t));
    return &msgs.back().first;
  };

  void remove_obsolete_messages() {

    typename std::list<std::pair<T,double>>::iterator it = msgs.begin();
    while (it != msgs.end()) {
      if (it->second < _last_time)
        it = msgs.erase(it);
      else 
        ++it;
    }
  }
};

#endif