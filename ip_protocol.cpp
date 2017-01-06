#include "ip_protocol.h"
void ip_protocol::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // reading initial ips

  next_internal = infinity;
  output = Event(0,5);
}

double ip_protocol::ta(double t) {
  return next_internal;
}

void ip_protocol::dint(double t) {

  if (!udp_datagram_in.empty()) {
    udp::Datagram d = udp_datagram_in.front();
    this->processUDPDatagram(d);
    udp_datagram_in.pop();
    next_internal = process_udp_datagram_time;
    return;
  }

  if (!link_packet_in.empty()) {
    ip::Packet p = link_packet_in.front();
    this->processIPPacket(p);
    link_packet_in.pop();
    next_internal = process_ip_packet_time;
    return;
  }
}

void ip_protocol::dext(Event x, double t) {
  switch(x.port) {
  case 0: // UDP protocol Datagram arrives
    udp_datagram_in.push(*(udp::Datagram*)x.value);
    break;
  case 1: // UDP protocol control message arrives
    udp_ctrl_in.push(*(udp::Control*)x.value);
    break;
  case 2: // ip packet arrives from link layer
    link_packet_in.push(*(ip::Packet*)x.value);
    break;
  case 3: // link layer control message arrives
    ntw_ctrl_in.push(*(link::Control*)x.value);
    break;
  default:
    printLog("[ERROR][ip protocol] invalid port ");
    printLog(std::to_string(x.port).c_str());
    printLog("\n");
    break;
  }

  // TODO: t is global time while next_internal it doesn't. must fix this bug
  if (next_internal < infinity)
    next_internal -= t;
  else if (this->queuedMsgs()) 
    next_internal = 0;
}

Event ip_protocol::lambda(double t) {
  return output;
}

void ip_protocol::exit() {}


/***********************************/
/********* HELPER METHODS **********/
/***********************************/

void udp_protocol::processIPPacket(ip::Packet p) {
  // TODO: implement this.
}

void udp_protocol::processUDPDatagram(udp::Datagram d) {
  ip::Packet p;
  p.data = d;
  // version 0100 (IPv4) IHL 0101 (5 no option field)
  // DSCP 0000 (currently not used) ECN 0000 (currently not used)
  p.header.vide = 0x4500;
  p.header.total_length = sizeof(id::Header) + d.psd_header.udp_lenght;
  p.header.ttlp = 0xFF00; // set ttl in hexa FF and protocol in zeros. TODO: check what should put in protocol
  p.header.src_ip = d.psd_header.src_ip;
  p.header.dest_ip = d.psd_header.dest_ip;
  p.header.calculate_checksum();

  this->processIPPacket(p);
}

bool ip_protocol::queuedMsgs() {
  return  !udp_ctrl_in.empty() ||
          !link_ctrl_in.empty() || 
          !udp_datagram_in.empty() || 
          !link_packet_in.empty();
}