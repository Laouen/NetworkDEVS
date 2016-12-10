#if !defined ip_h
#define ip_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

#include "abstract_types.h"

// network layer structures
namespace ip {

  enum Ctrl { ADD_IP, REMOVE_IP };

  struct Control {
    Ctrl request;
    std::string ip;

    Control() {}
    Control(const Control& o) {
      request = o.request;
      ip = o.ip;
    }
  };
}

#endif