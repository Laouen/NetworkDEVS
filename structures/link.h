#if !defined link_h
#define link_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>
#include <vector>

#include "abstract_types.h"
#include "ipv4.h"
#include "mac.h"
#include "ip.h"

#define IS_ARP_PACKET 0x00010000
#define ARP_TIMEOUT 900 // 900 seg = 15 min

namespace link {

  namespace arp {

    struct Entry {
      IPv4 ip;
      MAC mac;
      double timeout;

      Entry() {}
      Entry(const Entry& other) {
        ip = other.ip;
        mac = other.mac;
        timeout = other.timeout;
      }
    };

    struct Packet : abstract::Data {
      ushort Hardware_type;
      ushort Protocol_type;
      u_char HLen;
      u_char PLen;
      ushort Operation;
      MAC Source_Hardware_Address;
      IPv4 Source_Protocol_Address;
      MAC Target_Hardware_Address;
      IPv4 Target_Protocol_Address;

      Packet() {}
      Packet(const Packet& other) {
        Hardware_type = other.Hardware_type;
        Protocol_type = other.Protocol_type;
        HLen = other.HLen;
        PLen = other.PLen;
        Operation = other.Operation;
        Source_Hardware_Address = other.Source_Hardware_Address;
        Source_Protocol_Address = other.Source_Protocol_Address;
        Target_Hardware_Address = other.Target_Hardware_Address;
        Target_Protocol_Address = other.Target_Protocol_Address;
      }

      std::string as_string() const {
        std::string res = "ARP Packet:\n";
        res += "Hardware_type: " + std::to_string(Hardware_type) + "\n";
        res += "Protocol_type: " + std::to_string(Protocol_type) + "\n";
        res += "HLen: " + std::to_string((int)HLen) + "\n";
        res += "PLen: " + std::to_string((int)PLen) + "\n";
        res += "Operation: " + std::to_string(Operation) + "\n";
        res += "Source_Hardware_Address: " + Source_Hardware_Address.as_string() + "\n";
        res += "Source_Protocol_Address: " + Source_Protocol_Address.as_string() + "\n";
        res += "Target_Hardware_Address: " + Target_Hardware_Address.as_string() + "\n";
        res += "Target_Protocol_Address: " + Target_Protocol_Address.as_string();
        return res;
      }
    };
  }

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
      memcpy(payload,other.payload,1500);
      memcpy(interpacket_gat,other.interpacket_gat,sizeof(other.interpacket_gat));
    }

    void disableARPFlag() {
      preamble = preamble & 0xFFFEFFFF;
    }

    void enableARPFlag() {
      preamble = preamble & 0xFFFEFFFF;
      preamble = preamble | IS_ARP_PACKET;
    }

    void setPayload(const ip::Datagram& p) {
      memcpy(payload,&p,sizeof(p));
      disableARPFlag();
    }

    void setPayload(const arp::Packet& p) {
      memcpy(payload,&p,sizeof(p));
      enableARPFlag(); 
    }
  };

  struct Multiplexed_frame {
    Frame frame;
    ushort interface;

    Multiplexed_frame() {}
    Multiplexed_frame(const Multiplexed_frame& other) {
      frame = other.frame;
      interface = other.interface;
    }
  };

  enum Ctrl { ARP_QUERY, ARP_READY, SEND_PACKET, SEND_PACKET_FAILED };

  inline std::string to_string(Ctrl c) {
    std::string res;
    switch(c) {
    case link::Ctrl::ARP_QUERY: res = "ARP_QUERY"; break;
    case link::Ctrl::ARP_READY: res = "ARP_READY"; break;
    case link::Ctrl::SEND_PACKET: res = "SEND_PACKET"; break;
    case link::Ctrl::SEND_PACKET_FAILED: res = "SEND_PACKET_FAILED"; break;
    }
    return res;
  }

  struct Control {
    Ctrl request;
    ushort interface;
    IPv4 ip;
    ip::Datagram packet;

    Control() {}
    Control(const Control& other) {
      request = other.request;
      interface = other.interface;
      ip = other.ip;
      packet = other.packet;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const link::Frame& f) {
  os << "preamble: " << std::hex << f.preamble << std::dec << std::endl;
  os << "MAC_destination: " << f.MAC_destination << std::endl;
  os << "MAC_source: " << f.MAC_source << std::endl;
  os << "EtherType: " << f.EtherType << std::endl;
  os << "CRC: " << f.CRC << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const link::arp::Packet& p) {
  os << "Hardware_type: " << p.Hardware_type << std::endl;
  os << "Protocol_type: " << p.Protocol_type << std::endl;
  os << "HLen: " << (int)p.HLen << std::endl;
  os << "PLen: " << (int)p.PLen << std::endl;
  os << "Operation: " << p.Operation << std::endl;
  os << "Source_Hardware_Address: " << p.Source_Hardware_Address << std::endl;
  os << "Source_Protocol_Address: " << p.Source_Protocol_Address << std::endl;
  os << "Target_Hardware_Address: " << p.Target_Hardware_Address << std::endl;
  os << "Target_Protocol_Address: " << p.Target_Protocol_Address << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const link::Ctrl& c) {
  switch(c) {
  case link::Ctrl::ARP_QUERY: os << "ARP_QUERY"; break;
  case link::Ctrl::ARP_READY: os << "ARP_READY"; break;
  case link::Ctrl::SEND_PACKET: os << "SEND_PACKET"; break;
  case link::Ctrl::SEND_PACKET_FAILED: os << "SEND_PACKET_FAILED"; break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const link::Control& c) {
  os << "request: " << c.request << std::endl;
  os << "interface: " << c.interface << std::endl;
  os << "ip: " << c.ip << std::endl;
  os << "Datagram: " << std::endl << c.packet << std::endl;
  return os;
}

inline std::istream& operator>>(std::istream& is, link::Ctrl& c) {
  std::string a;
  is >> a;
  if (a == "ARP_QUERY") c = link::Ctrl::ARP_QUERY;
  if (a == "ARP_READY") c = link::Ctrl::ARP_READY;
  if (a == "SEND_PACKET") c = link::Ctrl::SEND_PACKET;
  if (a == "SEND_PACKET_FAILED") c = link::Ctrl::SEND_PACKET_FAILED;
  return is;
}

inline std::istream& operator>>(std::istream& is, link::Frame& f) {
  is >> f.preamble;
  is >> f.MAC_destination;
  is >> f.MAC_source;
  is >> f.EtherType;
  is >> f.CRC;
  return is;
}

// Only for testing purposes
inline std::istream& operator>>(std::istream& is, link::Control& c) {
  is >> c.request;
  is >> c.interface;
  is >> c.ip;
  return is;
}



#endif