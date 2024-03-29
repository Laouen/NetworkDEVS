#if !defined ip_protocol_h
#define ip_protocol_h

#include "template/protocol.h"

/* Related documentation
 *
 * https://www.iplocation.net/subnet-mask
 * distance-vector: RIP - peterson pag 243.
 * Routing: peterson 3.3 pag 240.
 * for a complete information about IPv4 header: https://en.wikipedia.org/wiki/IPv4
 * IPv4 routing documents
 * https://technet.microsoft.com/en-us/library/dd379495(v=ws.10).aspx // routing table
 * https://technet.microsoft.com/en-us/library/cc958831.aspx // used routing algorithm
 * https://developer.apple.com/library/content/documentation/NetworkingInternet/Conceptual/NetworkingConcepts/PacketRoutingandDelivery/PacketRoutingandDelivery.html
 * 
 */

class ip_protocol: public Protocol<udp::Segment, ip::Control, ip::Datagram, link::Control> { 

protected:

  //ARP structures
  std::list<ip::Forwarding_entry> forwarding_table;
  std::map<IPv4,std::queue<ip::Datagram>> arp_waiting_packets;

  // IP structures
  std::list<IPv4> host_ips;
  std::list<ip::Routing_entry> routing_table;

  /********** TIMES ***************/
  // in milliseconds
  double process_udp_segment_time = 70;
  double process_ip_datagram_time = 70;
  double process_link_control_time = 70;
  double send_frame_time = 10;

  /********* Protected methods *********/
  // Class state modifiers
  void routeIPDatagram(ip::Datagram);
  void arp(ip::Datagram, IPv4);
  void processLinkControl(link::Control);
  // Class state non modifiers
  ushort calculateChecksum(ip::Header) const;
  bool verifychecksum(ip::Header) const;
  bool matchesHostIps(IPv4) const;
  bool getBestRoute(IPv4, ip::Routing_entry&) const;
  bool isBestRoute(ip::Routing_entry, ip::Routing_entry) const;
  bool getInterface(IPv4, ushort&) const;

public:
	ip_protocol(const char *n): Protocol(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();

  virtual void dinternal(double) {}
  virtual void dexternal(double) {}
};

#endif