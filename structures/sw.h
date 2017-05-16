#if !defined sw_h
#define sw_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <stdlib.h> // abs
#include <list>

#include "mac.h"

/**
 * @namespace sw
 * 
 * This namespace is used to declare all the switches related datatypes.
 */
namespace sw {

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct sw::Forwarding_entry
   * 
   * @brief This structures declares an entry of the Datagram protocol forwading table.
   * @details The switches uses this datatype to fill their forwarding tables in order
   * to determine through what interfaces forward a link::Frame.
   * 
   */
  struct Forwarding_entry {
    MAC mac;
    ushort interface;

    /**
     * @brief Default constructor
     * @details Construct an empty instance.
     */
    Forwarding_entry() {}

    /**
     * @brief Copy constructor
     * @details Construct a copy of the instance passed as parameter in the new instance.
     * 
     * @param other The sw::Forwarding_entry to copy in the new instance.
     */
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