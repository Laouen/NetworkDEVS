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

  struct Frame : abstract::Data {
    MAC MAC_destination;
    MAC MAC_source;
    ushort EtherType;
    ip::Packet payload;
    unsigned long CRC;

    Frame() {}
    Frame(const Frame& other) {
      MAC_destination = other.MAC_destination;
      MAC_source = other.MAC_source;
      EtherType = other.EtherType;
      payload = other.payload;
      CRC = other.CRC;
    }
  };

  enum Ctrl { IDLE }; // TODO modify this enum correctly

  struct Control {
    Ctrl request;
  };
}

#endif