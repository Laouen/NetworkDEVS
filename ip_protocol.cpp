#include "ip_protocol.h"
void ip_protocol::init(double t,...) {

  // Logger init
  logger.setModuleName("IPv4");

  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // reading initial host ips
  int ip_amount = (int)va_arg(parameters,double);
  printLog("cantidad de hot ips: ");
  printLog(std::to_string(ip_amount).c_str());
  printLog("\n");
  for(int i=0;i<ip_amount;++i){
    host_ips.push_back(va_arg(parameters,char*));
  }

  for(std::list<IPv4>::iterator i = host_ips.begin(); i != host_ips.end(); ++i){
    printLog(i->as_string().c_str());
    printLog("\n");
  }

  // building routing table
  const char* file_path = va_arg(parameters,char*);
  Parser<ip::Routing_entry> parser(file_path);
  std::pair<double,ip::Routing_entry> parsed_line;
  if (parser.file_open()) {
    while(true) {
      try {
        parsed_line = parser.next_input();
        routing_table.push_back(parsed_line.second);
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    printLog("[IPv4] Error parsing routing table file.\n");
  }

  printLog("[IPv4] Routing table entries:\n");
  for (std::list<ip::Routing_entry>::iterator i = routing_table.begin(); i != routing_table.end(); ++i) {
    printLog(i->as_string().c_str());
    printLog("\n");
  }

  next_internal = infinity;
  output = Event(0,5);
}

double ip_protocol::ta(double t) {
  return next_internal;
}

void ip_protocol::dint(double t) {

  if (!udp_datagram_in.empty()) {
    udp::Datagram d = udp_datagram_in.front();
    this->processUDPDatagram(d,t);
    udp_datagram_in.pop();
    next_internal = process_udp_datagram_time;
    return;
  }

  if (!link_packet_in.empty()) {
    ip::Packet p = link_packet_in.front();
    this->processIPPacket(p,t);
    link_packet_in.pop();
    next_internal = process_ip_packet_time;
    return;
  }

  next_internal = infinity;
  output = Event(0,5);
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
    link_ctrl_in.push(*(link::Control*)x.value);
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

void ip_protocol::processIPPacket(ip::Packet p, double t) {
  ip::Routing_entry route;

  // The used algorithm is the microsoft routing process that specifies 7 steps
  // Step 1  
  if (!this->verifychecksum(p.header)) {
    // silent discard
    output = Event(0,5);
    return;
  }

  // Step 2
  if (this->matchesHostIps(p.header.dest_ip)) {
    // Delivering datagram to the next top layer
    output = Event(datagrams_out.push(p.data,t),0);
    return;
  }

  // Step 3  
  if (this->TTLisZero(p.header.ttlp)) {
    // silent discard. Some IP implementations send 
    // ICMP destination Unreachable-Network message to sender
    printLog("[IPv4] Discard packet: TTL zero\n");
    output = Event(0,5);
    return; 
  }

  // Step 4  
  p.header.ttlp = this->decreaseTTL(p.header.ttlp);
  // Recalculates checksum because the TTL field has being updated
  p.header.header_checksum = this->calculateChecksum(p.header);

  // Step 5  
  if (!this->getBestRoute(p.header.dest_ip, route)) {
    // silent discard
    printLog("[IPv4] Discard packet: No route for packet:\n");
    output = Event(0,5);
    return; 
  }

  // Step 6-7
  logger.info("Best route: " + route.as_string());
  this->sendPacket(p,route.nexthop,route.interface,t);
}

void ip_protocol::processUDPDatagram(udp::Datagram d, double t) {
  ip::Packet p;
  p.data = d;
  // version 0100 (IPv4) IHL 0101 (5 no option field)
  // DSCP 0000 (currently not used) ECN 0000 (currently not used)
  p.header.vide = 0x4500;
  p.header.total_length = sizeof(ip::Header) + d.psd_header.udp_lenght;
  p.header.ttlp = 0xFF00; // set ttl in hexa FF and protocol in zeros. TODO: check what should put in protocol
  p.header.src_ip = d.psd_header.src_ip;
  p.header.dest_ip = d.psd_header.dest_ip;
  p.header.header_checksum = this->calculateChecksum(p.header);

  this->processIPPacket(p,t);
}

bool ip_protocol::queuedMsgs() const {
  return  !udp_ctrl_in.empty() ||
          !link_ctrl_in.empty() || 
          !udp_datagram_in.empty() || 
          !link_packet_in.empty();
}

bool ip_protocol::TTLisZero(ushort ttlp) const {
  ushort ttl = ttlp & 0xFF00;
  return ttl == 0;
}

ushort ip_protocol::calculateChecksum(ip::Header header) const {
  // TODO: implement this method;
  return 0;
}

bool ip_protocol::verifychecksum(ip::Header header) const {
  // TODO: implement this method;
  return true;
}

bool ip_protocol::matchesHostIps(IPv4 dest_ip) const {
  for (std::list<IPv4>::const_iterator it = host_ips.cbegin(); it != host_ips.cend(); ++it) {
    if (dest_ip == *it) 
      return true;
  }
  return false;
}

ushort ip_protocol::decreaseTTL(ushort ttlp) const {
  ushort ttl = ttlp & 0xFF00;
  ttl--;
  ttl = ttl & 0xFFFF;
  return ttl & ttlp;
}

bool ip_protocol::getBestRoute(IPv4 dest_ip, ip::Routing_entry& route) const {
  bool found_route = false;
  for (std::list<ip::Routing_entry>::const_iterator it = routing_table.cbegin(); it != routing_table.cend(); ++it) {
    if (it->sameSubnet(dest_ip) && (!found_route || this->isBestRoute(*it,route))) {
      route = *it;
      found_route = true;
    }  
  }
  return found_route;
}

bool ip_protocol::isBestRoute(ip::Routing_entry current, ip::Routing_entry old) const {
  IPv4 longest = old.netmask | current.netmask;
  return (longest == current.netmask) || (current.metric < old.metric);
}

void ip_protocol::sendPacket(ip::Packet packet, IPv4 nexthop, IPv4 interface, double t) {
  // It sends a packet throw a control message to its next under layer.
  // FOR TESTING WE SEND IP PACKET DIRECTLY
  /*
  ip::Control c;
  c.packet = packet;
  c.nexthop = nexthop;
  c.interface = interface;
  output = Event(ip_control_out.push(c,t),3);
  */
  output = Event(ip_packet_out.push(packet,t),2);
}