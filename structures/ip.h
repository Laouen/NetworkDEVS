#if !defined ip_h
#define ip_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>
#include <vector>

#include "abstract_types.h"
#include "ipv4.h"

// network layer structures
namespace ip {

  struct Routing_entry {
    IPv4 network;
    IPv4 netmask;
    IPv4 nexthop;

    Routing_entry() {}
    Routing_entry(IPv4 other_network, IPv4 other_netmask, IPv4 other_nexthop) {
      network = other_network;
      netmask = other_netmask;
      nexthop = other_nexthop;
    }
    Routing_entry(const Routing_entry& other) {
      network = other.network;
      netmask = other.netmask;
      nexthop = other.nexthop;
    }

    bool same_subnet() {} // TODO: implement this method
  };

  enum Ctrl { ADD_IP, REMOVE_IP };

  struct Control {
    Ctrl request;
    IPv4 ip;

    Control() {}
    Control(const Control& o) {
      request = o.request;
      ip = o.ip;
    }
  };
}

#endif