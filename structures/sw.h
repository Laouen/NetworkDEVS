#if !defined sw_h
#define sw_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <stdlib.h> // abs
#include <list>

#include "mac.h"

namespace sw {
  struct Forwarding_entry {
    MAC mac;
    ushort interface;

    Forwarding_entry() {}
    Forwarding_entry(const Forwarding_entry& other) {
      mac = other.mac;
      interface = other.interface;
    }
  };
}

inline std::istream& operator>>(std::istream& is, sw::Forwarding_entry& e) {
  is >> e.mac >> e.interface;
  return is;
}

#endif