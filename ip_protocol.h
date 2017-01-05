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

  // comunication queues
  std::queue<udp::Control> udp_ctrl_in;
  std::queue<link::Control> link_ctrl_in;
  // data queues
  std::queue<udp::Datagram> udp_data_in;
  std::queue<link::Packet> link_data_in;

  std::vector<Routing_entry> routing_table;
  message_list<ip::Packet> datagrams_out;
  message_list<ip::Control> multiplexed_out;
  Event output;

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