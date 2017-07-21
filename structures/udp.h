#if !defined udp_h
#define udp_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

#include "abstract_types.h"
#include "ipv4.h"
#include "dns.h"

/**
 * @namespace udp
 * 
 * This namespace is used to declare all the UDP protocol related datatypes
 */
namespace udp {

  /**
   *
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct udp::PseudoHeader udp.h
   *  
   * @brief This struct declares an udp::Segment pseudo-header.
   * @details The pseudo-header has IP layer related information that is needed
   * in the UDP protocol due to the socket handling process where IP are used.
   * An udp::PseudoHeader has the next fields:
   * 1. src_ip: The local host IP 
   * 2. dest_ip: The remote host IP
   * 3. zeros: zeros
   * 4. protocol: The transport protocol number (currently not used)
   * 5. udp_lenght: The total udp::Segment length
   * 
   */
  struct PseudoHeader : abstract::Header {
    IPv4 src_ip;
    IPv4 dest_ip;
    u_char zeros = 0x0;
    u_char protocol = 0x0; // currently not used
    ushort udp_lenght = 0;

    /**
     * @brief Default constructor.
     * @details Construct an empty new instance
     */
    PseudoHeader() {}

    /**
     * @brief Copy constructor
     * @details Construct a new instance with a copy of instance passed as parameter.
     * 
     * @param o An udp::Segment to copy its values in the new instance.
     */
    PseudoHeader(const PseudoHeader& o) {
      src_ip = o.src_ip;
      dest_ip = o.dest_ip;
      udp_lenght = o.udp_lenght;
      protocol = o.protocol;
      zeros = o.zeros;
    }

    /**
     * @return The size in bytes of the instance.
     */
    ushort size() {
      return 20; // this is the current size of header for checksum
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
    const char* c_str() {
      char* foo = new char[this->size()];
      memcpy(foo, src_ip.ip, 8);
      memcpy(&foo[8], dest_ip.ip, 8);
      memcpy(&foo[16], &udp_lenght, 2);
      memcpy(&foo[18], &zeros, 1);
      memcpy(&foo[19], &protocol, 1);
      return foo;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct udp::Header udp.h
   * 
   * @brief This struct declares an udp::Segment header.
   * @details An udp::Header has the next fields:
   * 1. src_port: The local host socket port of the udp::Segment.
   * 2. dest_port: The local host socket ip of the udp::Segment.
   * 3. length: The total length of the udp::Segment.
   * 4. checksum: The checksum used to verify the udp::Segment has correctly arrived.
   * 
   */
  struct Header : abstract::Header {
    ushort src_port = 0;
    ushort dest_port = 0;
    ushort length = 0;
    ushort checksum = 0;

    /**
     * @brief Default constructor.
     * @details Construct an empty new instance.
     */
    Header () {}

    /**
     * @brief Copy constructor
     * @details Construct a new instance with a copy of the instance passed as parameter.
     * 
     * @param o An udp::Header to copy in the new instance.
     */
    Header(const Header& o) {
      src_port = o.src_port;
      dest_port = o.dest_port;
      length = o.length;
      checksum = o.checksum;
    }

    /**
     * @return Returns the instance size in bytes
     */
    ushort size() {
      return 6; // this is the current size of header for checksum
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
    const char* c_str() {
      char* foo = new char[this->size()];
      memcpy(foo, &src_port, 2);
      memcpy(&foo[2], &dest_port, 2);
      memcpy(&foo[4], &length, 2);
      return foo;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @brief This struct declares an udp::Segment to send through the network.
   * 
   * @details A udp::Segment has an udp::pseudoHeader, an udp::Header and a 
   * payload of length 512 bytes in char format.
   * 
   */
  struct Segment : abstract::Data {
    PseudoHeader psd_header;
    Header header;
    char payload[512]; // UDP size limit

    /**
     * @brief Default constructor
     * @details Construct a new empty instance.
     */
    Segment() {
      for(int i=0; i<512; ++i) payload[i] = 0x00;
    }

    /**
     * @brief Copy constructor
     * 
     * @details Construct a new instance with a copy of the instance passed as parameter.
     * 
     * @param o An udp::Segment to copy in the new instance.
     */
    Segment(const Segment& o) {
      psd_header = o.psd_header;
      header = o.header;
      memcpy(&payload[0], &o.payload[0], 512);
    }

    /**
     * @return The instance size in bytes.
     */
    ushort headers_size() {
      return header.size()+psd_header.size();
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
    const char* headers_c_str() {
      char* foo = new char[this->headers_size()];
      const char* h = header.c_str(); 
      const char* ph = psd_header.c_str(); 
      memcpy(foo, h, header.size());
      memcpy(&foo[header.size()], ph, psd_header.size());
      delete[] h;
      delete[] ph;
      return foo;
    }

    /**
     * @brief Set a dns::Packet as the payload. 
     * @details For this purpose it uses the c_str dns::Packet method to get the 
     * a pointer to a memory block with the instance char representation.
     * 
     * @param packet The dns::Packet to set as payload.
     */
    void setPayload(const dns::Packet& packet) {
      const char* payload_str = packet.c_str();
      memcpy(&payload[0],payload_str,packet.size());
      delete[] payload_str;
    }
  };

  /**
   * @enum Ctrl
   * 
   * the interface Ctrl was taken from: http://www.chuidiang.com/clinux/sockets/sockets_simp.php#sockets
   */
  enum Ctrl { SUCCESS, INVALID_SOCKET, INVALID_SOCKET_STATE, 
              CONNECT, BIND, READ_FROM, RECV_FROM, READ, RECV, WRITE_TO, SEND_TO, WRITE, SEND, CLOSE };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct udp::Control udp.h
   * 
   * @brief This structures handles the control comunication between the application layer and the 
   * UDP Protocol.
   * 
   * @details Every command as connect, bind, read, etc are requested from the application layer
   * using an udp::Control struct, specifying the socket and application ID. 
   * An udp::Control has the next fields:
   * 1. local_port: The local unsigned short port of the socket that the control message refers to.
   * 2. remote_port: The remote unsigned short port of the socket that the control message refers to (only for a connected socket).
   * 3. local_ip: The local IPv4 ip of the socket that the control message refers to.
   * 4. remote_ip: The remote IPv4 ip of the socket that the control message refers to (only for a connected socket).
   * 5. app_id: The application ID bound to the refered socket.
   * 6. request: The udp::Ctrl with the request command.
   * 7. packet: If the request command is send/write/send_to/write_to, this fields must have the dns::Packet to send through the network.
   * 
   */
  struct Control : abstract::Data {
    Ctrl request;
    unsigned int app_id;
    dns::Packet packet;

    // socket info
    ushort local_port;
    ushort remote_port;
    IPv4 local_ip;
    IPv4 remote_ip;

    /**
     * @brief Defualt constructor.
     * 
     * @details Construct a new empty instance.
     */
    Control() {}

    /**
     * @brief Copy constructor
     * @details Construct a new instance with a copy of the instance passed as parameter.
     * 
     * @param other The udp::Control instance to copy in the new instance.
     */
    Control(const Control& other) {
      request = other.request;
      app_id = other.app_id;
      packet = other.packet;
      local_port = other.local_port;
      remote_port = other.remote_port;
      local_ip = other.local_ip;
      remote_ip = other.remote_ip;
    }

    /**
     * @brief Constructor simple requests.
     * @details Construct a new udp::Control specifying only the request and 
     * app_ip fields. This can be used for requests where the rest of the 
     * information is not neded. 
     * 
     * @param int An int with the application ID.
     * @param other_request An udp::Ctrl with the request.
     */
    Control(unsigned int other_app_id, Ctrl other_request) {
      app_id = other_app_id;
      request = other_request;
    }

    /**
     * @brief Checks whether the udp::Control is a request from the appplication 
     * layer to the UDP protocol or not.
     * 
     * @details A request can be something else than a command from the application 
     * layer to be excecuted by the UDP protocol, as for example the 
     * udp::Ctrl::INVALID_SOCKET request that is a control messages to tell the 
     * application layer the specified socket is invalid for the send command.
     * 
     * @return True if it is an application request, False otherwise.
     */
    bool isAppRequest() const {
      switch(request) {
      case CONNECT: 
      case BIND: 
      case READ_FROM: 
      case RECV_FROM:
      case READ: 
      case RECV:
      case WRITE_TO:
      case SEND_TO:
      case WRITE: 
      case SEND: 
      case CLOSE:
        return true;
        break;
      default:
        return false;
        break;
      }
    }

    /**
     * @brief Prints in a human redable format the value of the udp::Control in
     * a std::string.
     * 
     * @return A std::string with the printed value.
     */
    std::string as_string() const {
      std::stringstream ss;
      ss << "app_id: " << app_id << std::endl;
      ss << "Ctrl: " << request << std::endl;
      if (isAppRequest()) {
        ss << "local_port: " << local_port << std::endl;
        ss << "local_ip: " << local_ip << std::endl;
      }
      
      if (request == Ctrl::WRITE ||
          request == Ctrl::WRITE_TO ||
          request == Ctrl::SEND ||
          request == Ctrl::SEND_TO) {
        ss << packet.as_string() << std::endl;
      } else if ( request == Ctrl::CONNECT ||
                  request == Ctrl::SEND_TO ||
                  request == Ctrl::WRITE_TO ||
                  request == Ctrl::RECV_FROM ||
                  request == Ctrl::READ_FROM) {
        ss << "remote_port: " << remote_port << std::endl;
        ss << "remote_ip: " << remote_ip << std::endl;
      }
      return ss.str();
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const udp::Ctrl& c) {
  switch(c) {
  case udp::Ctrl::SUCCESS:               os << "SUCCESS";              break;
  case udp::Ctrl::INVALID_SOCKET:        os << "INVALID_SOCKET";       break;
  case udp::Ctrl::INVALID_SOCKET_STATE:  os << "INVALID_SOCKET_STATE"; break;
  case udp::Ctrl::CONNECT:               os << "CONNECT";              break;
  case udp::Ctrl::BIND:                  os << "BIND";                 break;
  case udp::Ctrl::READ_FROM:             os << "READ_FROM";            break;
  case udp::Ctrl::RECV_FROM:             os << "RECV_FROM";            break;
  case udp::Ctrl::READ:                  os << "READ";                 break;
  case udp::Ctrl::RECV:                  os << "RECV";                 break;
  case udp::Ctrl::WRITE_TO:              os << "WRITE_TO";             break;
  case udp::Ctrl::SEND_TO:               os << "SEND_TO";              break;
  case udp::Ctrl::WRITE:                 os << "WRITE";                break;
  case udp::Ctrl::SEND:                  os << "SEND";                 break;
  case udp::Ctrl::CLOSE:                 os << "CLOSE";                break;
  }
  return os;
}

inline std::istream& operator>>(std::istream& is, udp::Ctrl& c) {
  std::string ch;
  is >> ch;
  if      (ch == "SUCCESS")               c = udp::Ctrl::SUCCESS;
  else if (ch == "INVALID_SOCKET")        c = udp::Ctrl::INVALID_SOCKET;
  else if (ch == "INVALID_SOCKET_STATE")  c = udp::Ctrl::INVALID_SOCKET_STATE;
  else if (ch == "CONNECT")               c = udp::Ctrl::CONNECT;
  else if (ch == "BIND")                  c = udp::Ctrl::BIND;
  else if (ch == "READ_FROM")             c = udp::Ctrl::READ_FROM;
  else if (ch == "RECV_FROM")             c = udp::Ctrl::RECV_FROM;
  else if (ch == "READ")                  c = udp::Ctrl::READ;
  else if (ch == "RECV")                  c = udp::Ctrl::RECV;
  else if (ch == "WRITE_TO")              c = udp::Ctrl::WRITE_TO;
  else if (ch == "SEND_TO")               c = udp::Ctrl::SEND_TO;
  else if (ch == "WRITE")                 c = udp::Ctrl::WRITE;
  else if (ch == "SEND")                  c = udp::Ctrl::SEND;
  else if (ch == "CLOSE")                 c = udp::Ctrl::CLOSE;
  return is;
}

inline std::istream& operator>>(std::istream& is, udp::Header& h) {
  is >> h.src_port;
  is >> h.dest_port;
  is >> h.length;
  is >> h.checksum;
  return is;
}

inline std::istream& operator>>(std::istream& is, udp::PseudoHeader& ph) {
  is >> ph.src_ip;
  is >> ph.dest_ip;
  is >> ph.udp_lenght;
  return is;
}

// This method is for testing purpose only, it does not load any payload
inline std::istream& operator>>(std::istream& is, udp::Segment& s) {
  is >> s.psd_header;
  is >> s.header;
  return is;
}

inline std::ostream& operator<<(std::ostream& os, const udp::Control& m) {
  os << "app_id: " << m.app_id << std::endl;
  os << "Ctrol: " << m.request << std::endl;
  if (m.isAppRequest()) {
    os << "local_port: " << m.local_port << std::endl;
    os << "local_ip: " << m.local_ip << std::endl;
  }
  
  if (m.request == udp::Ctrl::CONNECT ||
      m.request == udp::Ctrl::SEND_TO ||
      m.request == udp::Ctrl::WRITE_TO ||
      m.request == udp::Ctrl::RECV_FROM ||
      m.request == udp::Ctrl::READ_FROM) {
    os << "remote_port: " << m.remote_port << std::endl;
    os << "remote_ip: " << m.remote_ip << std::endl;
  }

  if (m.request == udp::Ctrl::WRITE ||
      m.request == udp::Ctrl::WRITE_TO ||
      m.request == udp::Ctrl::SEND ||
      m.request == udp::Ctrl::SEND_TO) {
    os << "packet: " << std::endl; 
    os << m.packet << std::endl;
  } 

  return os;
}

inline std::istream& operator>>(std::istream& is, udp::Control& m) {
  is >> m.request >> m.app_id;
  if (m.isAppRequest()) {
    is >> m.local_port >> m.local_ip;

    // If remote address is necessary
    if (m.request == udp::Ctrl::CONNECT ||
        m.request == udp::Ctrl::SEND_TO ||
        m.request == udp::Ctrl::WRITE_TO ||
        m.request == udp::Ctrl::RECV_FROM ||
        m.request == udp::Ctrl::READ_FROM) {
      is >> m.remote_port >> m.remote_ip;
    }

    // If sending data
    if (m.request == udp::Ctrl::WRITE ||
        m.request == udp::Ctrl::WRITE_TO ||
        m.request == udp::Ctrl::SEND ||
        m.request == udp::Ctrl::SEND_TO) {
      is >> m.packet;
    }
  }
  return is;
}

inline std::ostream& operator<<(std::ostream& os, const udp::Header& h) {
  os << "src_port: " << h.src_port << std::endl;
  os << "dest_port: " << h.dest_port << std::endl;
  os << "length: " << h.length << std::endl;
  os << "checksum: " << h.checksum << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const udp::PseudoHeader& ph) {
  os << "src_ip: " << ph.src_ip << std::endl;
  os << "dest_ip: " << ph.dest_ip << std::endl;
  os << "zeros: " << (int)ph.zeros << std::endl;
  os << "protocol: " << (int)ph.protocol << std::endl;
  os << "udp_lenght: " << ph.udp_lenght << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const udp::Segment& d) {
  os << "psd_header: " << std::endl << d.psd_header << std::endl;
  os << "header: " << std::endl << d.header << std::endl;
  dns::Packet p(&d.payload[0]);
  os << "payload: " << std::endl << p << std::endl;
  return os;
}

#endif