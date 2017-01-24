//CPP:networkDEVS/udp_protocol.cpp
#if !defined udp_protocol_h
#define udp_protocol_h

#include "layer.h"

/**
 * references: 
 * - https://www.ietf.org/rfc/rfc768.txt
 * - https://en.wikibooks.org/wiki/Communication_Networks/TCP_and_UDP_Protocols
 * - https://tools.ietf.org/html/rfc1071
 *
 */

// app layer send packet throw the control channel (port 1) using udp::Control structs
// udp layer deliver app:Packet throw the data channel (part 0) using udp::Multiplexed_packet
class udp_protocol: public Layer<udp::Multiplexed_packet, udp::Control, udp::Datagram, ip::Control> {

  // TODO: change map<port,map<ip,socket>> to map<ip,map<port,socket>>
  // is most likely to have one or a lower amount of IPs than ports.
  std::map<ushort,std::map<IPv4,udp::Socket>> sockets; // max socket amount 2^17-1 = 65536
  std::vector<IPv4> ips;

  /********** TIMES ***************/
  double add_rm_ip_time = 0.001; // takes 1 milliseconds to deliver data to the up layer
  double delivering_time = 0.1; // takes 100 milliseconds to deliver data to the up layer
  double app_ctrl_time = 0.01; // takes 1 milliseconds to bind a socket

  /************ Helper methods **************/
  void processDatagram(const udp::Datagram&, double );
  void processUDPCtrl(const udp::Control&, double);
  void sendData(const app::Packet&, const udp::Socket&, double);
  void sendDataTo(const app::Packet&, const udp::Socket&, ushort, IPv4, double);
  bool verifyChecksum(udp::Datagram d) const;
  ushort calculateChecksum(udp::Datagram d) const;
  void processNtwCtrl(const ip::Control&);
  bool existentIP(IPv4);
  bool existentSocket(ushort, IPv4);
  bool validAppSocket(ushort, IPv4, int);

public:
	udp_protocol(const char *n): Layer(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();
	virtual void dinternal(double);
};

#endif