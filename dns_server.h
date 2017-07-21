//CPP:networkDEVS/dns_server.cpp
#if !defined dns_server_h
#define dns_server_h

#include "template/protocol.h"
#include <cmath>

/* Documentation
 *
 * 
 */

class dns_server: public Protocol<dns::DomainName, int, dns::Packet, udp::Control, dns::Packet> { 

protected:

  int next_id;
  bool bind;
  bool start_reading;
  bool recursive_allowed;
  IPv4 local_ip;
  IPv4 server_ip;
  std::list<dns::ResourceRecord> cached_RRs;
  std::list<dns::ResourceRecord> authoritative_RRs;
  std::list<dns::Zone> zone_servers;
  std::list<dns::Packet> app_requests;
  std::list<dns::Packet> host_requests;

  /********** TIMES ***************/
  // In milliseconds
  double process_dns_query = 85;
  double process_dns_response = 85;

  /********* Protected methods *********/
  void processDomainName(dns::DomainName domain);
  void sendTo(const dns::Packet& p, IPv4 server_ip, ushort server_port);
  void processDNSPacket(dns::Packet packet);
  void setAsResponse(dns::Packet& packet);
  void directAnswer(dns::Packet& packet);
  void deliverAnswer(const dns::Packet& packet);
  void setAuthoritativeFlag(dns::Packet& packet, dns::DomainName d);
  void removePacket(ushort id);
  dns::Packet getPacket(ushort id);
  void updateCache(double t);
  dns::Zone getBestMatch(dns::DomainName d);
  dns::Packet QueryPacket(dns::DomainName d);
  
  bool isAppRequest(ushort id) const;
  bool isHostRequest(ushort id) const;
  bool existRR(const dns::DomainName& d) const;
  dns::ResourceRecord getRR(const dns::DomainName& d);

public:
  dns_server(const char *n): Protocol(n) {};
  void init(double, ...);
  double ta(double t);
  Event lambda(double);
  void exit();

  virtual void dinternal(double);
  virtual void dexternal(double);
};

#endif