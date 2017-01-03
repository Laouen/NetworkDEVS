#if !defined socket_h
#define socket_h

#include <string>
#include "ipv4.h"

namespace udp {
  struct Socket {

    enum Status { IDLE,CONNECTED,BOUND,LISTENING };

    ushort local_port = 0;
    ushort remote_port = 0;
    IPv4 local_ip;
    IPv4 remote_ip;

    int app_id;
    Status state = Status::IDLE;
    
    // flags
    bool reading = false;
    bool reading_from = false;

    // Constructors
    Socket() {
      app_id = 0;
    }

    Socket(int aid) {
      app_id = aid;
    }

    Socket(ushort o_local_port, IPv4 o_local_ip, int o_app_id) {
      app_id = o_app_id;
      local_port = o_local_port;
      local_ip = o_local_ip;
    }

    Socket(ushort o_local_port, IPv4 o_local_ip, ushort o_remote_port, IPv4 o_remote_ip, int o_app_id) {
      app_id = o_app_id;
      local_port = o_local_port;
      local_ip = o_local_ip;
      remote_port = o_remote_port;
      remote_ip = o_remote_ip;
    }

    Socket(const Socket& o) {
      app_id = o.app_id;
      local_port = o.local_port;
      local_ip = o.local_ip;
      remote_port = o.remote_port;
      remote_ip = o.remote_ip;
      state = o.state;
      reading = o.reading;
      reading_from = o.reading_from;
    }

    bool connected() {
      return state == Status::CONNECTED;
    }

    bool bound() {
      return state == Status::BOUND;
    }

    void startReading() {
      reading = true;
    }

    void startReadingFrom(ushort port, IPv4 ip) {
      remote_port = port;
      remote_ip = ip;
      reading_from = true;
    }

    void stopReading() {
      reading = false;
      reading_from = false;

      if (state == Status::BOUND) {
        remote_port = 0;
        remote_ip = IPv4();
      }
    }

    bool validLocalAddress(ushort port, IPv4 ip) {
      return  port == local_port && ip == local_ip;
    }

    bool validRemoteAddress(ushort port, IPv4 ip) {
      return port == remote_port && ip == remote_ip;
    }

    bool accept(ushort local_port, IPv4 local_ip, ushort remote_port, IPv4 remote_ip) {
      if (validLocalAddress(local_port,local_ip)) {
        if (bound()) {
          return  reading || 
                  (
                    reading_from && 
                    validRemoteAddress(remote_port,remote_ip)
                  );
        }

        if (connected() && reading && validRemoteAddress(remote_port,remote_ip)) {
          return true;
        }
      }

      return false;
    }
  };
}

#endif