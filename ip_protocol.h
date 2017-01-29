#if !defined ip_protocol_h
#define ip_protocol_h

#include "layer.h"

/* Documentation
 *
 * https://www.iplocation.net/subnet-mask
 * distance-vector: RIP - peterson pag 243.
 * Routing: peterson 3.3 pag 240.
 * for a complete information about IPv4 header: https://en.wikipedia.org/wiki/IPv4
 * IPv4 routing documents
 * https://technet.microsoft.com/en-us/library/dd379495(v=ws.10).aspx // routing table
 * https://technet.microsoft.com/en-us/library/cc958831.aspx // routing algorithm used
 * https://developer.apple.com/library/content/documentation/NetworkingInternet/Conceptual/NetworkingConcepts/PacketRoutingandDelivery/PacketRoutingandDelivery.html
 * 
 */

class ip_protocol: public Layer<udp::Datagram, ip::Control, link::Frame, ip::arp::Packet > { 

protected:

  //ARP MAC structures
  MAC mac;
  std::map<IPv4,ip::Forwarding_entry> forwarding_table;
  std::map<IPv4,std::queue<ip::Packet>> arp_waiting_packets;
  std::queue<link::Frame> arp_ready_packet;

  // IP structures
  IPv4 ip;
  std::list<IPv4> reserved_ips;
  std::list<ip::Routing_entry> routing_table;

  /********** TIMES ***************/
  double process_udp_datagram_time = 0.001;
  double process_ip_packet_time = 0.001;
  double process_arp_packet_time = 0.001;
  double send_frame_time = 0.001;

  /********* Protected methods *********/
  // Class state modifiers
  ip::Packet getIpPacket(const link::Frame&);
  void routeIPPacket(ip::Packet, double);
  void sendPacket(ip::Packet packet, MAC nexthop_mac);
  void arp(ip::Packet, IPv4, double);
  void sendArpQuery(IPv4, double);
  void processARPPacket(ip::arp::Packet, double);
  void cacheSourceMAC(MAC, IPv4);
  // Class state non modifiers
  ushort calculateChecksum(ip::Header) const;
  bool verifychecksum(ip::Header) const;
  bool matchesHostIps(IPv4) const;
  bool getBestRoute(IPv4, ip::Routing_entry&) const;
  bool isBestRoute(ip::Routing_entry, ip::Routing_entry) const;

public:
	ip_protocol(const char *n): Layer(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();

  virtual void dinternal(double) {}
};

#endif