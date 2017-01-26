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

  unsigned long IS_IP_PACKET = 0x00010000; 

  struct Frame : abstract::Data {
    unsigned long preamble;
    MAC MAC_destination;
    MAC MAC_source;
    ushort EtherType;
    char payload[1500];
    unsigned long CRC;
    u_char interpacket_gat[12];

    Frame() {
      preamble = 0x00000000;
    }
    Frame(const Frame& other) {
      preamble = other.preamble;
      MAC_destination = other.MAC_destination;
      MAC_source = other.MAC_source;
      EtherType = other.EtherType;
      CRC = other.CRC;
      memcpy(payload,other.payload,sizeof(other.payload));
      memcpy(interpacket_gat,other.interpacket_gat,sizeof(other.interpacket_gat));
    }

    void setAsIpPacket() {
      preamble = preamble & 0xFFFEFFFF;
      preamble = preamble | IS_IP_PACKET;
    }

    void setAsARPPacket() {
      preamble = preamble & 0xFFFEFFFF;
    }

    void setPayload(const ip::Packet& p) {
      memcpy(payload,&p,sizeof(p));
      setAsIpPacket();
    }

    void setPayload(const ip::arp::Packet& p) {
      memcpy(payload,&p,sizeof(p));
      setAsARPPacket(); 
    }
  };

  enum Ctrl { IDLE }; // TODO modify this enum correctly

  struct Control {
    Ctrl request;
  };
}

inline std::ostream& operator<<(std::ostream& os, const link::Frame& f) {
  os << "preamble: " << std::hex << f.preamble << std::dec << std::endl;
  os << "MAC_destination: " << f.MAC_destination << std::endl;
  os << "MAC_source: " << f.MAC_source << std::endl;
  os << "EtherType: " << f.EtherType << std::endl;
  os << "CRC: " << f.CRC << std::endl;
  if (f.preamble & link::IS_IP_PACKET) {
    ip::Packet* p = (ip::Packet*)f.payload;
    os << "payload (IP Packet): " << std::endl << *p << std::endl;
  } else {
    ip::arp::Packet* p = (ip::arp::Packet*)f.payload;
    os << "payload (ARP Packet): " << std::endl << *p << std::endl;
  }
  
  return os;
}

#endif