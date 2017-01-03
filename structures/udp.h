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

  enum Ctrl { SUCCESS,INVALID_SOCKET,INVALID_SOCKET_STATE,DELIVERED_PACKAGE };

  struct PseudoHeader : abstract::Header {
    IPv4 src_ip;
    IPv4 dest_ip;
    char zeros = 0;
    char protocol = 0; // currently not used
    ushort udp_lenght = 0;

    PseudoHeader() {}
    PseudoHeader(const PseudoHeader& o) {
      src_ip = o.src_ip;
      dest_ip = o.dest_ip;
      udp_lenght = o.udp_lenght;
      protocol = o.protocol;
      zeros = o.zeros;
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
  };

  struct Datagram : abstract::Data {
    PseudoHeader psd_header;
    Header header;
    app::Package payload;

    Datagram() {}
    Datagram(const Datagram& o) {
      psd_header = o.psd_header;
      header = o.header;
      payload = o.payload;
    }
  };

  struct Control : abstract::Data {
    int app_id;
    Ctrl control;
    app::Package data;

    Control() {}
    Control(int o_app_id, Ctrl o_control) {
      app_id = o_app_id;
      control = o_control;
    }
    
    Control(int o_app_id, const app::Package& o_data) {
      app_id = o_app_id;
      data = o_data;
      control = Ctrl::DELIVERED_PACKAGE;
    }
    
    Control(const Control& o) {
      app_id = o.app_id;
      data = o.data;
      control = o.control;
    }

    friend std::ostream& operator<<(std::ostream& os, const Control& m);
  };

  inline std::ostream& operator<<(std::ostream& os, const Ctrl& c) {
    switch(c) {
    case Ctrl::SUCCESS: os << "SUCCESS"; break;
    case Ctrl::INVALID_SOCKET: os << "INVALID_SOCKET"; break;
    case Ctrl::INVALID_SOCKET_STATE: os << "INVALID_SOCKET_STATE"; break;
    case Ctrl::DELIVERED_PACKAGE: os << "DELIVERED_PACKAGE"; break;
    }
    return os;
  }
  
  inline std::ostream& operator<<(std::ostream& os, const Control& m) {
    os << "app_id: " << m.app_id << std::endl;
    os << "Ctrol: " << m.control << std::endl;
    if (m.control == udp::Ctrl::DELIVERED_PACKAGE) {
      os << "Data: " << m.data << std::endl;
    }
    return os;
  }
}

#endif