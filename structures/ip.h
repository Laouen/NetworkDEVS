#if !defined ip_h
#define ip_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>
#include <vector>

#include "abstract_types.h"
#include "ipv4.h"
#include "udp.h"

// network layer structures
namespace ip {

  struct Header : abstract::Header {
    ushort vide; // Version,IHL,DSCP,ECN
    ushort total_length;
    ushort identification;
    ushort ff; // flag,fragment_offset
    ushort ttlp; // time to live,protocol
    ushort header_checksum;
    IPv4 src_ip;
    IPv4 dest_ip;
    // not implementing opntions field

    Header() {}
    Header(const Header& other) {
      vide = other.vide;
      total_length = other.total_length;
      identification = other.identification;
      ff = other.ff;
      ttlp = other.ttlp;
      header_checksum = other.header_checksum;
      src_ip = other.src_ip;
      dest_ip = other.dest_ip;  
    }
    void calculate_checksum() {} // TODO: implement checksum
  };

  struct Packet : abstract::Data {
    Header header;
    udp::Datagram data;

    Packet() {}
    Packet(const Packet& other) {
      header = other.header;
      data = other.data;
    }
  };

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

    bool same_subnet(IPv4 des_ip) {
      IPv4 dest_subnet = des_ip & netmask; 
      IPv4 entry_subnet = network & netmask;
      return dest_subnet == entry_subnet;
    }
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