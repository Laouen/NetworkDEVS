#if !defined app_h
#define app_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

#include "abstract_types.h"

// Application layer structures
namespace app {

  typedef std::string Packet;
}

#endif