//CPP:networkDEVS/udp_protocol.cpp
#if !defined udp_protocol_h
#define udp_protocol_h

#include "template/protocol.h"

/**
 * references: 
 * - https://www.ietf.org/rfc/rfc768.txt
 * - https://en.wikibooks.org/wiki/Communication_Networks/TCP_and_UDP_Protocols
 * - https://tools.ietf.org/html/rfc1071
 *
 */

class udp_protocol: public Protocol<dns::Packet, udp::Control, udp::Segment, ip::Control> {

  std::map<ushort,std::map<IPv4,udp::Socket>> sockets;
  std::vector<IPv4> ips;

  /********** TIMES ***************/
  double add_rm_ip_time = 0.01;
  double delivering_time = 0.01;
  double app_ctrl_time = 0.01;

  /************ Helper methods **************/
  void processSegment(const udp::Segment&, double );
  void processUDPCtrl(const udp::Control&, double);
  void sendData(const dns::Packet&, const udp::Socket&, double);
  void sendDataTo(const dns::Packet&, const udp::Socket&, ushort, IPv4, double);
  bool verifyChecksum(udp::Segment d) const;
  ushort calculateChecksum(udp::Segment d) const;
  void processNtwCtrl(const ip::Control&);
  bool existentIP(IPv4);
  bool existentSocket(ushort, IPv4);
  bool validAppSocket(ushort, IPv4, int);

public:
	udp_protocol(const char *n): Protocol(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();
	virtual void dinternal(double);
};

#endif