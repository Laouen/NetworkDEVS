#if !defined udp_h
#define udp_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

#include "abstract_types.h"
#include "app.h"
#include "ipv4.h"

// transport layer structures
namespace udp {

  struct PseudoHeader : abstract::Header {
    IPv4 src_ip;
    IPv4 dest_ip;
    u_char zeros = 0x0;
    u_char protocol = 0x0; // currently not used
    ushort udp_lenght = 0;

    PseudoHeader() {}
    PseudoHeader(const PseudoHeader& o) {
      src_ip = o.src_ip;
      dest_ip = o.dest_ip;
      udp_lenght = o.udp_lenght;
      protocol = o.protocol;
      zeros = o.zeros;
    }

    ushort size() {
      return 20; // this is the current size of header for checksum
    }

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

  struct Header : abstract::Header {
    ushort src_port;
    ushort dest_port;
    ushort length;
    ushort checksum;

    Header () {}
    Header(const Header& o) {
      src_port = o.src_port;
      dest_port = o.dest_port;
      length = o.length;
      checksum = o.checksum;
    }

    ushort size() {
      return 6; // this is the current size of header for checksum
    }

    const char* c_str() {
      char* foo = new char[this->size()];
      memcpy(foo, &src_port, 2);
      memcpy(&foo[2], &dest_port, 2);
      memcpy(&foo[4], &length, 2);
      return foo;
    }
  };

  struct Datagram : abstract::Data {
    PseudoHeader psd_header;
    Header header;
    char payload[100]; // TODO: put the correct size

    Datagram() {}
    Datagram(const Datagram& o) {
      psd_header = o.psd_header;
      header = o.header;
      memcpy(payload,o.payload,sizeof(payload));
    }

    ushort headers_size() {
      return header.size()+psd_header.size();
    }

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
  };

  struct Multiplexed_packet : abstract::Data {
    unsigned int app_id;
    app::Packet packet;

    Multiplexed_packet() {}
    Multiplexed_packet(unsigned int other_app_id, app::Packet other_packet) {
      app_id = other_app_id;
      packet = other_packet;
    }
  };

  // the interface Ctrl was taken from: http://www.chuidiang.com/clinux/sockets/sockets_simp.php#sockets
  enum Ctrl { SUCCESS, INVALID_SOCKET, INVALID_SOCKET_STATE, 
              CONNECT, BIND, READ_FROM, RECV_FROM, READ, RECV, WRITE_TO, SEND_TO, WRITE, SEND, CLOSE };

  struct Control : abstract::Data {
    Ctrl request;
    unsigned int app_id;
    app::Packet packet;

    // socket info
    ushort local_port;
    ushort remote_port;
    IPv4 local_ip;
    IPv4 remote_ip;

    Control() {}
    Control(const Control& other) {
      request = other.request;
      app_id = other.app_id;
      packet = other.packet;
      local_port = other.local_port;
      remote_port = other.remote_port;
      local_ip = other.local_ip;
      remote_ip = other.remote_ip;
    }
    Control(unsigned int other_app_id, Ctrl other_request) {
      app_id = other_app_id;
      request = other_request;
    }

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
        ss << "packet: " << packet << std::endl;
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

inline std::ostream& operator<<(std::ostream& os, const udp::Control& m) {
  os << "app_id: " << m.app_id << std::endl;
  os << "Ctrol: " << m.request << std::endl;
  if (m.isAppRequest()) {
    os << "local_port: " << m.local_port << std::endl;
    os << "local_ip: " << m.local_ip << std::endl;
  }
  
  if (m.request == udp::Ctrl::WRITE ||
      m.request == udp::Ctrl::WRITE_TO ||
      m.request == udp::Ctrl::SEND ||
      m.request == udp::Ctrl::SEND_TO) {
    os << "packet: " << m.packet << std::endl;
  } else if ( m.request == udp::Ctrl::CONNECT ||
              m.request == udp::Ctrl::SEND_TO ||
              m.request == udp::Ctrl::WRITE_TO ||
              m.request == udp::Ctrl::RECV_FROM ||
              m.request == udp::Ctrl::READ_FROM) {
    os << "remote_port: " << m.remote_port << std::endl;
    os << "remote_ip: " << m.remote_ip << std::endl;
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
      std::getline(is,m.packet); // read until eol as string
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

inline std::ostream& operator<<(std::ostream& os, const udp::Datagram& d) {
  os << "psd_header: " << std::endl << d.psd_header << std::endl;
  os << "header: " << std::endl << d.header << std::endl;
  os << "payload: " << d.payload << std::endl;
  return os;
}

#endif