#if !defined app_h
#define app_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

#include "abstract_types.h"
#include "ipv4.h"

// Application layer structures
namespace app {
  
  // the interface Ctrl was taken from: http://www.chuidiang.com/clinux/sockets/sockets_simp.php#sockets
  enum Ctrl { CONNECT, BIND, READ_FROM, RECV_FROM, READ, RECV, WRITE_TO, SEND_TO, WRITE, SEND, CLOSE };

  typedef std::string Packet;

  struct Control : abstract::Data {
    Ctrl request;
    
    // local info
    int app_id;
    ushort port;
    IPv4 ip;
    // remote info
    ushort remote_port;
    IPv4 remote_ip;
    
    Packet data;

    Control() {}
    Control(const Control& o) {
      app_id = o.app_id;
      request = o.request;
      port = o.port;
      ip = o.ip;
      data = o.data;
      remote_ip = o.remote_ip;
      remote_port = o.remote_port;
    }

    friend std::istream& operator>>(std::istream& is, Control& c);
  };
  
  inline std::ostream& operator<<(std::ostream& os, const Ctrl& p) {
    switch (p) {
    case Ctrl::CONNECT: os << "CONNECT"; break;
    case Ctrl::BIND: os << "BIND"; break;
    case Ctrl::READ_FROM: os << "READ_FROM"; break;
    case Ctrl::RECV_FROM: os << "RECV_FROM"; break;
    case Ctrl::READ: os << "READ"; break;
    case Ctrl::RECV: os << "RECV"; break;
    case Ctrl::WRITE_TO: os << "WRITE_TO"; break;
    case Ctrl::SEND_TO: os << "SEND_TO"; break;
    case Ctrl::WRITE: os << "WRITE"; break;
    case Ctrl::SEND: os << "SEND"; break;
    case Ctrl::CLOSE: os << "CLOSE"; break;
    }
    return os;
  }

  inline std::istream& operator>>(std::istream& is, Ctrl& c) {
    std::string a;
    is >> a;
    if (a == "CONNECT") c = Ctrl::CONNECT;
    else if (a == "BIND") c = Ctrl::BIND;
    else if (a == "READ_FROM") c = Ctrl::READ_FROM;
    else if (a == "RECV_FROM") c = Ctrl::RECV_FROM;
    else if (a == "READ") c = Ctrl::READ;
    else if (a == "RECV") c = Ctrl::RECV;
    else if (a == "WRITE_TO") c = Ctrl::WRITE_TO;
    else if (a == "SEND_TO") c = Ctrl::SEND_TO;
    else if (a == "WRITE") c = Ctrl::WRITE;
    else if (a == "SEND") c = Ctrl::SEND;
    else if (a == "CLOSE") c = Ctrl::CLOSE;
    else {
      throw std::exception();
    }
    return is;
  }

  inline std::istream& operator>>(std::istream& is, Control& c) {
    is >> c.request;
    is >> c.app_id;
    is >> c.port;
    is >> c.ip;

    // If remote address is necessary
    if (c.request == Ctrl::CONNECT ||
        c.request == Ctrl::SEND_TO ||
        c.request == Ctrl::WRITE_TO ||
        c.request == Ctrl::RECV_FROM ||
        c.request == Ctrl::READ_FROM) {
      is >> c.remote_port;
      is >> c.remote_ip;
    }

    // If sending data
    if (c.request == Ctrl::WRITE ||
        c.request == Ctrl::WRITE_TO ||
        c.request == Ctrl::SEND ||
        c.request == Ctrl::SEND_TO) {
      std::getline(is,c.data); // read until eol as string
    }

    return is;
  }
}

#endif