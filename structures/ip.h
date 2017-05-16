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

/**
 * @namespace ip
 * 
 * This namespace to use for all the IP protocol datatypes
 */
namespace ip {

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct ip::Header ip.h
   * 
   * @brief This struct implement an IPv4 Datagram header.
   * @details An ip::Header has the next fields:
   * 1. vide: unsigned short that stores the version,IHL,DSCP and ECN.
   * 2. total_length: unsigned short that stores the total ip::Datagram length.
   * 3. identification: unsigned short that stores the identification.
   * 4. ff: unsigned short that stores the flags and fragment_offset.
   * 5. ttlp: unsigned short that stores the TTL and the Protocol.
   * 6. header_checksum: unsigned short that stores the header checksum.
   * 7. src_ip: IPv4 that stores the src IP.
   * 8. dest_ip: IPv4 that stores the dest IP.
   */
  struct Header : abstract::Header {
    ushort vide;
    ushort total_length;
    ushort identification;
    ushort ff;
    ushort ttlp;
    ushort header_checksum;
    IPv4 src_ip;
    IPv4 dest_ip;
    // not implementing opntions field

    /**
     * @brief Default constructor.
     * @details Construct an empty instance.
     */
    Header() {}

    /**
     * @brief Copy constructor
     * @details Construct a new instance from a copy of an existent one.
     * 
     * @param other A ip::Header to copy in the initialization of the new instance.
     */
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

    /**
     * @return The size of the instance in bytes.
     */
    int size() const {
      return sizeof(vide) +
              sizeof(total_length) +
              sizeof(identification) +
              sizeof(ff) +
              sizeof(ttlp) +
              sizeof(header_checksum) + 
              dest_ip.size() +
              src_ip.size();
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     *
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
    const char* c_str() const {
      char* foo = new char[this->size()];
      int i = 0;
      
      memcpy(&foo[i], &vide, sizeof(vide)); 
      i += sizeof(vide);
      memcpy(&foo[i], &total_length, sizeof(total_length)); 
      i += sizeof(total_length);
      memcpy(&foo[i], &identification, sizeof(identification)); 
      i += sizeof(identification);
      memcpy(&foo[i], &ff, sizeof(ff)); 
      i += sizeof(ff);
      memcpy(&foo[i], &ttlp, sizeof(ttlp)); 
      i += sizeof(ttlp);
      memcpy(&foo[i], &header_checksum, sizeof(header_checksum)); 
      i += sizeof(header_checksum);
      memcpy(&foo[i], dest_ip.ip, dest_ip.size()); 
      i += dest_ip.size();
      memcpy(&foo[i], src_ip.ip, src_ip.size());
      
      return foo;
    }

    /**
     * @return Returns the size of the ip::Header without the checksum size.
     */
    int checksum_size() const {
      return this->size() - sizeof(header_checksum);
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @details This methods does not copy in the memory block the checksum field.
     *
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
    const char* checksum_c_str() const {
      char* foo = new char[this->size()];
      int i = 0;
      
      memcpy(&foo[i], &vide, sizeof(vide)); 
      i += sizeof(vide);
      memcpy(&foo[i], &total_length, sizeof(total_length)); 
      i += sizeof(total_length);
      memcpy(&foo[i], &identification, sizeof(identification)); 
      i += sizeof(identification);
      memcpy(&foo[i], &ff, sizeof(ff)); 
      i += sizeof(ff);
      memcpy(&foo[i], &ttlp, sizeof(ttlp)); 
      i += sizeof(ttlp);
      memcpy(&foo[i], dest_ip.ip, dest_ip.size()); 
      i += dest_ip.size();
      memcpy(&foo[i], src_ip.ip, src_ip.size());
      
      return foo;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct ip::Datagram
   * 
   * @brief This class represent an ip::Datagram to send through the network.
   * @details It has the next fields:
   * 1. header: the ip::Header.
   * 2. data: The udp::Segment that is encapsulated to send through the network.
   */
  struct Datagram : abstract::Data {
    Header header;
    udp::Segment data;

    /**
     * @brief Default constructor.
     * @details It construct a new empty instance.
     */
    Datagram() {}

    /**
     * @brief Copy constructor
     * @details It construct a new instance with a copy of the ip::Datagram 
     * passed as parameter.
     * 
     * @param other An ip::Datagram to copy in the new instance.
     */
    Datagram(const Datagram& other) {
      header = other.header;
      data = other.data;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct ip::Routing_entry
   * 
   * @brief Represent an entry of the routing table of the ip protocol.
   * @details The ip::Routing_entry has the next field:
   * 1. network: a IPv4 containing the subnet of the remote host.
   * 2. netmask: a IPv4 containing the mask that indicate what bits of the network represent the network of the remote host.
   * 3. nexthope: a IPv4 containing the ip of the reachable nexthope where to send the ip::Datagram.
   * 4. metric: a int with a metric of how close is the remote host using the nexthope of the entry as the way to reach it.
   * 5. purpose: a std::string that specifies a description of the entry.
   */
  struct Routing_entry {
    IPv4 network;
    IPv4 netmask;
    IPv4 nexthop; // Same as gateway
    int metric;
    std::string purpose;

    /**
     * @brief Default constructor.
     * @details Construct a new empty instance.
     */
    Routing_entry() {}

    /**
     * @brief Constructor
     * @details It construct a new instance seting all its fields.
     * 
     * @param other_network The nextwork to set.
     * @param other_netmask The netmask to set. 
     * @param other_nexthop The nexthop to set.
     * @param other_metric The metric to set.
     * @param other_purpose The purpose to set.
     */
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

    /**
     * @brief Copy constructor
     * @details Construct a new instance with a copy of the one passed as parameter.
     * 
     * @param other An ip::Routing_entry to construct the new instance as a copy of it.
     */
    Routing_entry(const Routing_entry& other) {
      network   = other.network;
      netmask   = other.netmask;
      nexthop   = other.nexthop;
      metric    = other.metric;
      purpose   = other.purpose;
    }

    /**
     * @brief Cheks whether the other remote host is in the implicit this->network or not.
     * 
     * @param des_ip The IPv4 of the remote host to check if is in the same subnet. 
     * @return True if des_ip is in the same subnate of this->network.
     */
    bool sameSubnet(IPv4 des_ip) const {
      IPv4 dest_subnet = des_ip & netmask; 
      IPv4 entry_subnet = network & netmask;
      return dest_subnet == entry_subnet;
    }

    /**
     * @brief Prints in a std::string a human redable version of the ip::Routing_entry.
     * 
     * @return A std::string with the value printed on it.
     */
    std::string as_string() const {
      std::string res = "| ";
      res += network.as_string();
      for(ushort i=0;i<16-network.as_string().size();++i);
      res += " | ";
      res += netmask.as_string();
      for(ushort i=0;i<16-netmask.as_string().size();++i);
      res += " | ";
      res += nexthop.as_string();
      for(ushort i=0;i<16-nexthop.as_string().size();++i);
      res += " | ";
      res += std::to_string(metric);
      for(ushort i=0;i<5-std::to_string(metric).size();++i);
      res += " | ";
      res += purpose;
      for(ushort i=0;i<30-purpose.size();++i);
      res += " | ";
      return res;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct ip::Forwarding_entry
   * 
   * @brief Represent an entry of the forwarding table of the ip protocol.
   * @details The ip::Forwarding_entry has the next field:
   * 1. network: a IPv4 containing the subnet of the next hope.
   * 2. netmask: a IPv4 containing the mask that indicate what bits of the network represent the network of the next hope.
   * 3. interface: an unsigned short with the interface where to send the ip::Datagram to reach the next hope.
   */
  struct Forwarding_entry {
    IPv4 network;
    IPv4 netmask;
    ushort interface;

    /**
     * @brief Default constructor.
     * @details Construct a new empty instance.
     */
    Forwarding_entry() {}
    
    /**
     * @brief Copy constructor.
     * @details Construct a new instance with a copy of the one passed as parameter.
     * 
     * @param other The ip::Forwarding_entry used to construct the new copy.
     */
    Forwarding_entry(const Forwarding_entry& other) {
      network = other.network;
      netmask = other.netmask;
      interface = other.interface;
    }

    /**
     * @brief Cheks whether the other remote host is in the implicit this->network or not.
     * 
     * @param des_ip The IPv4 of the remote host to check if is in the same subnet. 
     * @return True if des_ip is in the same subnate of this->network.
     */
    bool sameSubnet(IPv4 des_ip) const {
      IPv4 dest_subnet = des_ip & netmask; 
      IPv4 entry_subnet = network & netmask;
      return dest_subnet == entry_subnet;
    }

    /**
     * @brief Prints in a std::string a human redable version of the ip::Forwarding_table.
     * 
     * @return A std::string with the value printed on it.
     */
    std::string as_string() const {
      std::string res = "| ";
      res += network.as_string();
      for(ushort i=0;i<16-network.as_string().size();++i);
      res += " | ";
      res += netmask.as_string();
      for(ushort i=0;i<16-netmask.as_string().size();++i);
      res += " | ";
      res += std::to_string(interface);
      for(ushort i=0;i<16-std::to_string(interface).size();++i);
      res += " | ";
      return res;
    }
  };

  /**
   * @enum ip::Ctrl
   * @brief Valid values of a ip::Control request.
   */
  enum Ctrl { ADD_IP, REMOVE_IP, ROUTING_ERROR };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct ip::Control ip.h
   * 
   * @brief This struct is used to send control messages to the UDP protocol in the 
   * near up layer.
   */
  struct Control {
    Ctrl request;
    IPv4 ip;

    /**
     * @brief Default constructor.
     * @details Construct a new empty instance.
     */
    Control() {}

    /**
     * @brief Constructor from ip::Ctrl
     * @details Construct a new instance with the request passed as parameter.
     * 
     * @param c The ip::Ctrl with the request value.
     */
    Control(Ctrl c) : request(c) {}

    /**
     * @brief Copy constructor
     * @details Construct a new instance from a copy of the one passed as parameter.
     * 
     * @param o The ip::Control to copy in the new instance.
     */
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

inline std::ostream& operator<<(std::ostream& os, const ip::Datagram& p) {
  os << "Header: " << std::endl << p.header << std::endl;
  os << "data: " << std::endl << p.data << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const ip::Routing_entry& e) {
  os << e.network << " | " << e.netmask << " | " << e.nexthop << " | ";
  os << e.metric << " | " << e.purpose;
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

inline std::istream& operator>>(std::istream& is, ip::Ctrl& c) {
  std::string a;
  is >> a;
  if (a == "ADD_IP") c = ip::Ctrl::ADD_IP; return is;
  if (a == "REMOVE_IP") c = ip::Ctrl::REMOVE_IP; return is;
  if (a == "ROUTING_ERROR") c = ip::Ctrl::ROUTING_ERROR; return is;
  return is;
}

inline std::istream& operator>>(std::istream& is, ip::Control& c) {
  is >> c.request;
  is >> c.ip;
  return is;
}

inline std::ostream& operator<<(std::ostream& os, const ip::Ctrl& c) {
  switch(c) {
  case ip::Ctrl::ADD_IP: os << "ADD_IP"; break;
  case ip::Ctrl::REMOVE_IP: os << "REMOVE_IP"; break;
  case ip::Ctrl::ROUTING_ERROR: os << "ROUTING_ERROR"; break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const ip::Control& c) {
  os << "Request: " << c.request << std::endl;
  if (c.request != ip::Ctrl::ROUTING_ERROR)
    os << "ip: " << c.ip;
  return os;
}

inline std::istream& operator>>(std::istream& is, ip::Header& h) {
  is >> h.vide;
  is >> h.total_length;
  is >> h.identification;
  is >> h.ff;
  is >> h.ttlp;
  is >> h.header_checksum;
  is >> h.src_ip;
  is >> h.dest_ip;
  return is;
}

inline std::istream& operator>>(std::istream& is, ip::Datagram& d) {
  is >> d.header;
  is >> d.data;
  return is;
}

#endif