//CPP:NetworkDEVS/ip_protocol.cpp
//CPP:NetworkDEVS/ip_host_protocol.cpp
#if !defined ip_host_protocol_h
#define ip_host_protocol_h

#include "ip_protocol.h"

class ip_host_protocol : public ip_protocol {

  void processIPPacket(ip::Packet);
  void processUDPDatagram(udp::Datagram);

public:
  ip_host_protocol(const char *n): ip_protocol(n) {};
  virtual void dinternal(double);
};

#endif