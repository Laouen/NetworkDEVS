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


/**
 * @namespace link
 * 
 * This namespace is used to declare all the datatypes related to the link layer module.
 */
namespace link {

  /**
   * @namespace arp
   * 
   * This namespace is used to declare all the datatypes related to the ARP protocol
   */
  namespace arp {

    /**
     * 
     * @author Laouen Louan Mayal Belloli
     * @date 14 May 2017
     * 
     * @struct link::arp::Entry link.h
     * 
     * @brief This struct represent an entry in the ARP cache Table.
     * @details A link::arp::Entry has the next fields:
     * 1. ip: The IPv4 to get the MAC Address.
     * 2. mac: The already obtained MAC address of ip.
     * 3. timeout: The cache timeout in seconds before remove the entry from the ARP cache table. 
     */
    struct Entry {
      IPv4 ip;
      MAC mac;
      double timeout;

      /**
       * @brief Default constructor
       * @details Construct a new empty instance.
       */
      Entry() {}

      /**
       * @brief Copy instance
       * @details Construct a new copy of the instances passed as parameter.
       * 
       * @param other The link::arp::Entry to copy in the new instance.
       */
      Entry(const Entry& other) {
        ip = other.ip;
        mac = other.mac;
        timeout = other.timeout;
      }
    };

    /**
     * 
     * @author Laouen Louan Mayal Belloli
     * @date 17 May 2017
     * 
     * @struct link::arp::Packet link.h
     * 
     * @brief This struct implements an ARP packet to send a query through the subnet
     * in order to get the response with the MAC address of the node with asigned IPv4 ip.
     * 
     * @details The fields of a link::arp::Packet are the next:
     * Hardware_type: a ushort (1 for MAC address type).
     * Protocol_type: a ushort (0x0800 for IPv4 type).
     * HLen: a u_char with the hardware type length.
     * PLen: a u_char with the protocol type length.
     * Operation: a ushort (1 for query, 0 for response).
     * Source_Hardware_Address: The source hot MAC.
     * Source_Protocol_Address: The source hot IPv4.
     * Target_Hardware_Address: The target host MAC.
     * Target_Protocol_Address: The target host IPv4.
     * 
     */
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

      /**
       * @brief Default constructor
       * @details Construct a new empty instance
       */
      Packet() {}

      /**
       * @brief Copy instance
       * @details Construct a new copy of the instances passed as parameter.
       * 
       * @param other The link::arp::Packet to copy in the new instance.
       */
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

      /**
       * @brief Print the link::arp::Packet in a human redable format on a std::string.
       * @return A std::string with the printed value.
       */
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

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct link::Frame link.h
   * 
   * @brief Implement a L2 frame.
   * @details A L2 Frame contains the next fields:
   * preamble: an unsigned long with all the required flags.
   * MAC_destination: The MAC Address of the destination host.
   * MAC_source: The MAC Address of the source host.
   * EtherType: Type of the fisical protocol (0 for Ethernet).
   * payload: a char[1500] with the ip::Datagram or ARP Paket to send.
   * interpacket_gat: a char[12] used to avoid solve some networks colitions problems. 
   */
  struct Frame : abstract::Data {
    unsigned long preamble;
    MAC MAC_destination;
    MAC MAC_source;
    ushort EtherType;
    char payload[1500];
    unsigned long CRC;
    u_char interpacket_gat[12];

    /**
     * @brief Default constructor.
     * @details Construct a new empty instance.
     */
    Frame() {

      ip::Datagram d;
      this->setPayload(d);
      preamble = 0x00000000;
    }

    /**
     * @brief Copy constructor.
     * @details Construct a copy of the instance passed as parameter.
     * 
     * @param other The link::Frame to copy in the new instance.
     */
    Frame(const Frame& other) {
      preamble = other.preamble;
      MAC_destination = other.MAC_destination;
      MAC_source = other.MAC_source;
      EtherType = other.EtherType;
      CRC = other.CRC;
      memcpy(payload,other.payload,1500);
      memcpy(interpacket_gat,other.interpacket_gat,sizeof(other.interpacket_gat));
    }

    /**
     * @brief It set the preamble ARP flag as desabled to tell that the L2 frame
     * carries an ip::Datagram.
     */
    void disableARPFlag() {
      preamble = preamble & 0xFFFEFFFF;
    }

    /**
     * @brief It set the preamble ARP flag as enables to tell that the L2 frame
     * carries an ARP Packet.
     * @details [long description]
     */
    void enableARPFlag() {
      preamble = preamble & 0xFFFEFFFF;
      preamble = preamble | IS_ARP_PACKET;
    }

    /**
     * @brief Sets an ip::Datagram as the payload and desables the ARP flags.
     */
    void setPayload(const ip::Datagram& p) {
      memcpy(payload,&p,sizeof(p));
      disableARPFlag();
    }

    /**
     * @brief Sets a link::arp::Packet as the payload and enables the ARP flags.
     */
    void setPayload(const arp::Packet& p) {
      memcpy(payload,&p,sizeof(p));
      enableARPFlag(); 
    }
  };

  /**
   * 
   * @enum link::Ctrl
   * 
   * Valid values for a link::Control message, the link::Control struct
   * is used to implement the comunication between the link layer and the ip layer.
   */
  enum Ctrl { ARP_QUERY, ARP_READY, SEND_PACKET, SEND_PACKET_FAILED };

  /**
   * @brief Prints a link::Ctrl in a std::String
   * @return A std::string with the printed value
   */
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

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct link::Control
   * 
   * @brief This struct is used to allow comunication between the io layer and 
   * ther link layer.
   * @details A link::Control has the next fields:
   * 1. request: a link::Ctrol that indicates the purpose of the message.
   * 2. interface: an unsigned short that indicate the interface where is conected the link module that is being part of the comunication process.
   * 3. ip: If the request is link::Ctrl::ARP_QUERY, this fields contains the ip to ve resolved.
   * 4. packet: If the request is link::Ctrl::SEND_PACKET, this fields contains the ip::Datagram to send.
   */
  struct Control {
    Ctrl request;
    ushort interface;
    IPv4 ip;
    ip::Datagram packet;

    /**
     * @brief Default constructor.
     * @details Construct a new empty instance.
     */
    Control() {}

    /**
     * @brief Copy constructor
     * @details Construct a copy of the instance passed as parameter in the new instance.
     * 
     * @param other The link::Control to copy in the new instance.
     */
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
  if (c.request == link::Ctrl::SEND_PACKET)
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
  c.packet.header.vide = 0;
  c.packet.header.total_length = 0;
  c.packet.header.identification = 0;
  c.packet.header.ff = 0;
  c.packet.header.ttlp = 50;
  c.packet.header.header_checksum = 0;
  c.packet.data.psd_header.src_ip = "1.0.0.1";
  c.packet.data.psd_header.dest_ip = "1.0.0.2";
  c.packet.data.psd_header.zeros = 0;
  c.packet.data.psd_header.protocol = 0;
  c.packet.data.psd_header.udp_lenght = 0;
  c.packet.data.header.src_port = 80;
  c.packet.data.header.dest_port = 8080;
  c.packet.data.header.length = 0;
  c.packet.data.header.checksum = 0;
  return is;
}



#endif