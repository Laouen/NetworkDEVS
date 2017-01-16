#include "ip_protocol.h"
void ip_protocol::init(double t,...) {

  // TODO: implement same subnet comparizon to know if the destination is in the same
  // subnet or not. If so, the packet is directly send to destination host. If not, the
  // packet is sent the subnet router. Peterson page 222.

  // TODO: packet fragmentation handler.Peterson page 213

  // TODO: Check IP packet format from Peterson page 209.
  // UPD protocol field number is 17.

  // Note: Currently packet are delivered multiple times as TTL allows becouse there isn't layer two
  // implementation and therefor, all packet are multicasted and returns to the sender how re send the packet
  // affter decreasing TTL field.

  // TODO: implement header checksum

  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters,char*);
  logger.setModuleName("IPv4 " + module_name);

  // reading initial host ips
  int ip_amount = (int)va_arg(parameters,double);
  logger.info("cantidad de hot ips: " + std::to_string(ip_amount));
  for(int i=0;i<ip_amount;++i){
    host_ips.push_back(va_arg(parameters,char*));
  }

  logger.info("Initial host ips:");
  for(std::list<IPv4>::iterator i = host_ips.begin(); i != host_ips.end(); ++i){
    logger.info(i->as_string());
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
    logger.error("Error parsing routing table file.");
  }

  logger.info("Routing table entries:");
  for (std::list<ip::Routing_entry>::iterator i = routing_table.begin(); i != routing_table.end(); ++i) {
    logger.info(i->as_string());
  }

  next_internal = infinity;
  output = Event(0,5);
}

double ip_protocol::ta(double t) {
  return next_internal;
}

void ip_protocol::dint(double t) {

  last_transition = t;

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
    logger.error("Invalid port.");
    break;
  }

  if (next_internal < infinity)
    next_internal -= (t-last_transition);
  else if (this->queuedMsgs()) 
    next_internal = 0;

  last_transition = t;
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
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("ip packet: " + dest_ip.as_string());
  // The used algorithm is the microsoft routing process that specifies 7 steps
  // Step 1
  if (!this->verifychecksum(p.header)) {
    // silent discard
    logger.info("Deliver to up layer packet with dest_ip: "
                + dest_ip.as_string());
    output = Event(0,5);
    return;
  }

  // Step 2
  if (this->matchesHostIps(dest_ip)) {
    // Delivering datagram to the next top layer
    output = Event(datagrams_out.push(p.data,t),0);
    return;
  }

  // Step 3  
  if (this->TTLisZero(p.header.ttlp)) {
    // silent discard. Some IP implementations send 
    // ICMP destination Unreachable-Network message to sender
    logger.info("Discard packet: TTL zero for packet with dest_ip: " 
                + dest_ip.as_string());
    output = Event(0,5);
    return; 
  }

  // Step 4  
  p.header.ttlp = this->decreaseTTL(p.header.ttlp);
  // Recalculates checksum because the TTL field has being updated
  p.header.header_checksum = this->calculateChecksum(p.header);

  // Step 5  
  if (!this->getBestRoute(dest_ip, route)) {
    // silent discard
    logger.info("Discard packet: No route for packet with dest_ip: " 
                + dest_ip.as_string());
    output = Event(0,5);
    return; 
  }

  // Step 6-7
  logger.info("Best route for packet with dest_ip: " 
              + dest_ip.as_string() + " is: " + route.as_string());
  this->sendPacket(p,route.nexthop,route.interface,t);
}

void ip_protocol::processUDPDatagram(udp::Datagram d, double t) {
  ip::Packet p;
  p.data = d;
  // version 0100 (IPv4) IHL 0101 (5 no option field)
  // DSCP 0000 (currently not used) ECN 0000 (currently not used)
  p.header.vide = 0x4500;
  p.header.total_length = sizeof(ip::Header) + d.psd_header.udp_lenght;
  // set ttl in hexa FF and protocol in zeros. 
  // TODO: check what should put in protocol
  p.header.ttlp = 0xFF00;
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
  ushort ttl = ttlp >> 8;
  logger.debug(std::to_string(ttl));
  logger.debug((ttl == 0) ? "is zero" : "isn't zero");
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
    if (dest_ip == *it) {
      return true;
    }
  }
  return false;
}

ushort ip_protocol::decreaseTTL(ushort ttlp) const {
  std::stringstream sstream;
  sstream << std::hex << ttlp;
  logger.debug("Start TTL: " + sstream.str());
  
  ushort ttl = ttlp >> 8;
  --ttl;
  ttl = ttl << 8;
  ttlp = ttlp & 0x00FF;
  ttlp = ttl | ttlp;
  
  sstream.str("");
  sstream << std::hex << ttlp;
  logger.debug("Final TTL: " + sstream.str());
  return ttlp;
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