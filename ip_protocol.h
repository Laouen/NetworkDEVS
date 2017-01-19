#if !defined ip_protocol_h
#define ip_protocol_h

#include "simulator.h"
#include "event.h"
#include "stdarg.h"

#include <string>
#include <map>
#include <queue>
#include <vector>
#include <utility>
#include <iostream>
#include <limits>
#include <iomanip>

#include "libs/message_list.h"
#include "libs/parser.h"
#include "libs/logger.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/ipv4.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"
#include "structures/link.h"


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

class ip_protocol: public Simulator { 

protected:
  // Logger
  Logger logger;

  // comunication queues
  std::queue<udp::Control> udp_ctrl_in;
  std::queue<ip::arp::Packet> arp_in;
  // data queues
  std::queue<udp::Datagram> udp_datagram_in;
  std::queue<link::Frame> link_frame_in;

  //ARP structures
  MAC mac;
  IPv4 ip;
  std::map<IPv4,ip::Forwarding_entry> forwarding_table;
  std::map<IPv4,std::queue<ip::Packet>> arp_waiting_packets;
  std::queue<link::Frame> arp_ready_packet;

  std::list<IPv4> reserved_ips;
  std::list<ip::Routing_entry> routing_table;

  // Output structures
  message_list<udp::Datagram> datagrams_out;
  message_list<link::Frame> link_frame_out;
  message_list<ip::arp::Packet> arp_packet_out;
  message_list<ip::Control> ip_control_out;
  Event output;

  double next_internal;
  double last_transition;

  /********** TIMES ***************/
  double infinity = std::numeric_limits<double>::max();
  double process_udp_datagram_time = 0.001;
  double process_ip_packet_time = 0.001;
  double process_arp_packet_time = 0.001;
  double send_frame_time = 0.001;

  /********* Protected methods *********/
  // Class state modifiers
  void routeIPPacket(ip::Packet, double);
  void sendPacket(ip::Packet packet, MAC nexthop_mac);
  void arp(ip::Packet, IPv4, double);
  void sendArpQuery(IPv4, double);
  void processARPPacket(ip::arp::Packet, double);
  void cacheSourceMAC(MAC, IPv4);
  // Class state non modifiers
  bool queuedMsgs() const;
  ushort calculateChecksum(ip::Header) const;
  bool verifychecksum(ip::Header) const;
  bool matchesHostIps(IPv4) const;
  bool getBestRoute(IPv4, ip::Routing_entry&) const;
  bool isBestRoute(ip::Routing_entry, ip::Routing_entry) const;

public:
	ip_protocol(const char *n): Simulator(n) {};
	void init(double, ...);
	double ta(double t);
  void dext(Event , double );
  Event lambda(double);
  void exit();
};

#endif