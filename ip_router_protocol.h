//CPP:NetworkDEVS/ip_protocol.cpp
//CPP:NetworkDEVS/ip_router_protocol.cpp
#if !defined ip_router_protocol_h
#define ip_router_protocol_h

#include "ip_protocol.h"

class ip_router_protocol : public ip_protocol {

  void processIPDatagram(ip::Datagram);
  bool TTLisZero(ushort) const;
  ushort decreaseTTL(ushort) const;

public:
  ip_router_protocol(const char *n): ip_protocol(n) {};
  virtual void dinternal(double);
};

#endif