//CPP:networkDEVS/ip_host_protocol.cpp
#if !defined ip_host_protocol_h
#define ip_host_protocol_h

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

class ip_host_protocol: public Simulator { 

  // Logger
  Logger logger;

  // comunication queues
  std::queue<udp::Control> udp_ctrl_in;
  std::queue<link::Control> link_ctrl_in;
  // data queues
  std::queue<udp::Datagram> udp_datagram_in;
  std::queue<ip::Packet> link_packet_in;

  std::list<IPv4> host_ips;
  std::list<ip::Routing_entry> routing_table;
  message_list<udp::Datagram> datagrams_out;
  message_list<ip::Control> ip_control_out;
  message_list<ip::Packet> ip_packet_out;
  Event output;

  double next_internal;
  double last_transition;

  /********** TIMES ***************/
  double infinity = std::numeric_limits<double>::max();
  double process_udp_datagram_time = 0.001;
  double process_ip_packet_time = 0.001;

  /********* Private methods *********/
  // Class state modifiers
  void processIPPacket(ip::Packet, double);
  void routeIPPacket(ip::Packet, double);
  void processUDPDatagram(udp::Datagram, double);
  void sendPacket(ip::Packet, IPv4, IPv4, double);
  // Class state non modifiers
  bool queuedMsgs() const;
  ushort calculateChecksum(ip::Header h) const;
  bool verifychecksum(ip::Header) const;
  bool matchesHostIps(IPv4) const;
  bool getBestRoute(IPv4, ip::Routing_entry&) const;
  bool isBestRoute(ip::Routing_entry, ip::Routing_entry) const;

public:
	ip_host_protocol(const char *n): Simulator(n) {};
	void init(double, ...);
	double ta(double t);
	void dint(double);
	void dext(Event , double );
	Event lambda(double);
	void exit();
};

#endif