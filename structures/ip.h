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
#include "mac.h"
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

    int size() {
      return 26; // this is the current size of header for checksum
    }

    const char* c_str() {
      char* foo = new char[this->size()];
      memcpy(foo, &vide, 2);
      memcpy(&foo[2], &total_length, 2);
      memcpy(&foo[4], &identification, 2);
      memcpy(&foo[6], &ff, 2);
      memcpy(&foo[8], &ttlp, 2);
      memcpy(&foo[10], dest_ip.ip, 8);
      memcpy(&foo[18], src_ip.ip, 8);
      return foo;
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
    int metric;
    std::string purpose;

    Routing_entry() {}
    Routing_entry(IPv4 other_network,
                  IPv4 other_netmask,
                  IPv4 other_nexthop,
                  int other_metric,
                  std::string other_purpose) {
      network   = other_network;
      netmask   = other_netmask;
      nexthop   = other_nexthop;
      metric    = other_metric;
      purpose   = other_purpose;
    }
    Routing_entry(const Routing_entry& other) {
      network   = other.network;
      netmask   = other.netmask;
      nexthop   = other.nexthop;
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
      for(ushort i=0;i<16-network.as_string().size();++i) res += " ";
      res += "\t | ";
      res += netmask.as_string();
      for(ushort i=0;i<16-netmask.as_string().size();++i) res += " ";
      res += "\t | ";
      res += nexthop.as_string();
      for(ushort i=0;i<16-nexthop.as_string().size();++i) res += " ";
      res += "\t | ";
      res += std::to_string(metric);
      for(ushort i=0;i<5-std::to_string(metric).size();++i) res += " ";
      res += "\t | ";
      res += purpose;
      for(ushort i=0;i<30-purpose.size();++i) res += " ";
      res += "\t | ";
      return res;
    }
  };

  struct Forwarding_entry {
    IPv4 network;
    IPv4 netmask;
    ushort interface;

    Forwarding_entry() {}
    Forwarding_entry(const Forwarding_entry& other) {
      network = other.network;
      netmask = other.netmask;
      interface = other.interface;
    }

    bool sameSubnet(IPv4 des_ip) const {
      IPv4 dest_subnet = des_ip & netmask; 
      IPv4 entry_subnet = network & netmask;
      return dest_subnet == entry_subnet;
    }

    std::string as_string() const {
      std::string res = "| ";
      res += network.as_string();
      for(ushort i=0;i<16-network.as_string().size();++i) res += " ";
      res += "\t | ";
      res += netmask.as_string();
      for(ushort i=0;i<16-netmask.as_string().size();++i) res += " ";
      res += "\t | ";
      res += std::to_string(interface);
      for(ushort i=0;i<16-std::to_string(interface).size();++i) res += " ";
      res += "\t | ";
      return res;
    }
  };

  enum Ctrl { ADD_IP, REMOVE_IP, ROUTING_ERROR };

  struct Control {
    Ctrl request;
    IPv4 ip;

    Control() {}
    Control(Ctrl c) : request(c) {}
    Control(const Control& o) {
      request = o.request;
      ip = o.ip;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const ip::Header& h) {
  os << "vide: " << h.vide << std::endl;
  os << "total_length: " << h.total_length << std::endl;
  os << "identification: " << h.identification << std::endl;
  os << "ff: " << h.ff << std::endl;
  os << "ttlp: " << h.ttlp << std::endl;
  os << "header_checksum: " << h.header_checksum << std::endl;
  os << "src_ip: " << h.src_ip << std::endl;
  os << "dest_ip: " << h.dest_ip << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const ip::Packet& p) {
  os << "Header: " << std::endl << p.header << std::endl;
  os << "data: " << std::endl << p.data << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const ip::Routing_entry& e) {
  os << e.network << " \t\t | " << e.netmask << " \t\t | " << e.nexthop << " \t\t | ";
  os << e.metric << " \t\t | " << e.purpose;
  return os;
}

inline std::istream& operator>>(std::istream& is, ip::Routing_entry& e) {
  is >> e.network >> e.netmask >> e.nexthop >> e.metric;
  std::getline(is,e.purpose);
  return is;
}

inline std::istream& operator>>(std::istream& is, ip::Forwarding_entry& e) {
  is >> e.network >> e.netmask >> e.interface;
  return is;
}

#endif