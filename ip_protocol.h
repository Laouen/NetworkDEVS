//CPP:networkDEVS/ip_protocol.cpp
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

#include "libs/message_list.h"

#include "structures/abstract_types.h"
#include "structures/socket.h"
#include "structures/udp.h"
#include "structures/app.h"
#include "structures/ip.h"
#include "structures/ipv4.h"


/* Documentation
 *
 * https://www.iplocation.net/subnet-mask
 */

class ip_protocol: public Simulator { 

  // data queues1
  std::queue<udp::Datagram> datagram_in;

  // comunication queues
  std::queue<app::Control> app_ctrl_in;
  std::queue<ip::Control> ntw_ctrl_in;

  map<>
  message_list<udp::Datagram> datagrams_out;
  message_list<udp::Control> multiplexed_out;
  Event output;

  // TODO: change map<port,map<ip,socket>> to map<ip,map<port,socket>>
  // is most likely to have one or a lower amount of IPs than ports.
  std::map<ushort,std::map<IPv4,udp::Socket>> sockets; // max socket amount 2^16 = 65536
  std::vector<IPv4> ips;

  double next_internal;

  /********** TIMES ***************/
  double infinity = std::numeric_limits<double>::max();

public:
	ip_protocol(const char *n): Simulator(n) {};
	void init(double, ...);
	double ta(double t);
	void dint(double);
	void dext(Event , double );
	Event lambda(double);
	void exit();
};

#endif