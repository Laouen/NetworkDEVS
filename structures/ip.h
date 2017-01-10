#if !defined ip_h
#define ip_h

#include <stddef.h>
#include <iostream>
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
    IPv4 nexthop; // Same as gateway
    IPv4 interface;
    int metric;
    std::string purpose;

    Routing_entry() {}
    Routing_entry(IPv4 other_network,
                  IPv4 other_netmask,
                  IPv4 other_nexthop,
                  IPv4 other_interface,
                  int other_metric,
                  std::string other_purpose) {
      network   = other_network;
      netmask   = other_netmask;
      nexthop   = other_nexthop;
      interface = other_interface;
      metric    = other_metric;
      purpose   = other_purpose;
    }
    Routing_entry(const Routing_entry& other) {
      network   = other.network;
      netmask   = other.netmask;
      nexthop   = other.nexthop;
      interface = other.interface;
      metric    = other.metric;
      purpose   = other.purpose;
    }

    bool sameSubnet(IPv4 des_ip) const {
      IPv4 dest_subnet = des_ip & netmask; 
      IPv4 entry_subnet = network & netmask;
      return dest_subnet == entry_subnet;
    }

    std::string as_string() const {
      std::string res = "| ";
      res += network.as_string();
      for(int i=0;i<16-network.as_string().size();++i) res += " ";
      res += "\t | ";
      res += netmask.as_string();
      for(int i=0;i<16-netmask.as_string().size();++i) res += " ";
      res += "\t | ";
      res += nexthop.as_string();
      for(int i=0;i<16-nexthop.as_string().size();++i) res += " ";
      res += "\t | ";
      res += interface.as_string();
      for(int i=0;i<16-interface.as_string().size();++i) res += " ";
      res += "\t | ";
      res += std::to_string(metric);
      for(int i=0;i<5-std::to_string(metric).size();++i) res += " ";
      res += "\t | ";
      res += purpose;
      for(int i=0;i<30-purpose.size();++i) res += " ";
      res += "\t | ";
      return res;
    }

  };

  enum Ctrl { ADD_IP, REMOVE_IP, SEND_PACKET };

  struct Control {
    Ctrl request;
    IPv4 ip;
    Packet packet;
    IPv4 nexthop;
    IPv4 interface;

    Control() {}
    Control(const Control& o) {
      request = o.request;
      ip = o.ip;
      packet = o.packet;
      nexthop = o.nexthop;
      interface = o.interface;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const ip::Routing_entry& e) {
  os << e.network << " \t\t | " << e.netmask << " \t\t | " << e.nexthop << " \t\t | ";
  os << e.interface << " \t\t | " << e.metric << " \t\t | " << e.purpose;
  return os;
}

inline std::istream& operator>>(std::istream& is, ip::Routing_entry& e) {
  is >> e.network >> e.netmask >> e.nexthop >> e.interface >> e.metric;
  std::getline(is,e.purpose);
  /*
  std::string word;
  e.purpose = "";
  do {
    is >> word;
    e.purpose += " " + word;
  } while(e.purpose.back() != '"');
  e.purpose = e.purpose.substr(2, e.purpose.size() - 3);
  */
  return is;
}

#endif