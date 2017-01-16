//CPP:networkDEVS/udp_protocol.cpp
#if !defined udp_protocol_h
#define udp_protocol_h

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

#include "libs/message_list.h"
#include "libs/logger.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/ipv4.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"
#include "structures/link.h"

/**
 * TODO: use a correct documentation format
 * author Laouen Belloli
 *
 * references: 
 * - https://www.ietf.org/rfc/rfc768.txt
 * - https://en.wikibooks.org/wiki/Communication_Networks/TCP_and_UDP_Protocols
 * - https://tools.ietf.org/html/rfc1071
 *
 */


class udp_protocol: public Simulator {

  // Logger
  Logger logger;

  // data queues1
  std::queue<udp::Datagram> datagram_in;

  // comunication queues
  std::queue<app::Control> app_ctrl_in;
  std::queue<ip::Control> ntw_ctrl_in;

  message_list<udp::Datagram> datagrams_out;
  message_list<udp::Control> multiplexed_out;
  Event output;

  // TODO: change map<port,map<ip,socket>> to map<ip,map<port,socket>>
  // is most likely to have one or a lower amount of IPs than ports.
  std::map<ushort,std::map<IPv4,udp::Socket>> sockets; // max socket amount 2^16 = 65536
  std::vector<IPv4> ips;

  double next_internal;
  double last_transition;

  /********** TIMES ***************/
  double add_rm_ip_time = 0.001; // takes 1 milliseconds to deliver data to the up layer
  double delivering_time = 0.1; // takes 100 milliseconds to deliver data to the up layer
  double app_ctrl_time = 0.01; // takes 1 milliseconds to bind a socket
  double infinity = std::numeric_limits<double>::max();

  /************ Helper methods **************/
  void processDatagram(const udp::Datagram&, double );
  void processAppCtrl(const app::Control&, double);
  void sendData(const app::Packet&, const udp::Socket&, double);
  void sendDataTo(const app::Packet&, const udp::Socket&, ushort, IPv4, double);
  bool evaluateChecksum(const app::Packet&, ushort);
  ushort calculateChecksum(const char*, ushort);
  void processNtwCtrl(const ip::Control&);
  bool existentIP(IPv4);
  bool existentSocket(ushort, IPv4);
  bool validAppSocket(ushort, IPv4, int);
  bool queuedMsgs();

public:
	udp_protocol(const char *n): Simulator(n) {};
	void init(double, ...);
	double ta(double t);
	void dint(double);
	void dext(Event , double );
	Event lambda(double);
	void exit();
};

#endif