#if !defined link_h
#define link_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>
#include <vector>

#include "abstract_types.h"
#include "ipv4.h"

// network layer structures
namespace link {

  enum Ctrl { IDLE }; // TODO modify this enum correctly

  struct Control {
    Ctrl request;
  };
}

#endif