#include "ip_protocol.h"

// Note: The used algorithm is the microsoft ip routing process at a sender

void ip_protocol::init(double t,...) {

  // TODO: implement same subnet comparizon to know if the destination is in the same
  // subnet or not. If so, the packet is directly send to destination host. If not, the
  // packet is sent the subnet router. Peterson page 222. THIS MUST BE DONE IN LAYER 2

  // Note: Currently packet are delivered multiple times as TTL allows becouse there isn't layer two
  // implementation and therefor, all packet are multicasted and returns to the sender how re send the packet
  // affter decreasing TTL field.
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters,char*);
  logger.setModuleName("IPv4 " + module_name);

  // reading initial host ips
  int ip_amount = (int)va_arg(parameters,double);
  logger.info("ip amount: " + std::to_string(ip_amount));
  for(int i=0;i<ip_amount;++i){
    reserved_ips.push_back(va_arg(parameters,char*));
  }

  logger.info("ips:");
  for(std::list<IPv4>::iterator i = reserved_ips.begin(); i != reserved_ips.end(); ++i){
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

  logger.info("Routing table:");
  for (std::list<ip::Routing_entry>::iterator i = routing_table.begin(); i != routing_table.end(); ++i) {
    logger.info(i->as_string());
  }

  // building forwarding table
  file_path = va_arg(parameters,char*);
  Parser<ip::Forwarding_entry> parser_ft(file_path);
  std::pair<double,ip::Forwarding_entry> parsed_line;
  if (parser_ft.file_open()) {
    while(true) {
      try {
        parsed_line = parser_ft.next_input();
        forwarding_table.push_back(parsed_line.second);
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    logger.error("Error parsing routing table file.");
  }

  logger.info("Forwarding table:");
  for (std::list<ip::Forwarding_entry>::iterator i = forwarding_table.begin(); i != forwarding_table.end(); ++i) {
    logger.info(i->as_string());
  }

  next_internal = infinity;
  output = Event(0,5);
}

double ip_protocol::ta(double t) {
  return next_internal;
}

Event ip_protocol::lambda(double t) {
  return output;
}

void ip_protocol::exit() {}


/***********************************/
/********* HELPER METHODS **********/
/***********************************/

void ip_protocol::routeIPPacket(ip::Packet p, double t) {
  ip::Routing_entry route;
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("sending ip packet: " + dest_ip.as_string());
  
  if (!this->getBestRoute(dest_ip, route)) {
    logger.info("Discard packet: No route for packet with dest_ip: " + dest_ip.as_string());
    ip::Control m = ip::Control(ip::Ctrl::ROUTING_ERROR);
    output = Event(higher_layer_ctrl_out.push(m,t), 1);
    return; 
  }

  // Step 6-7
  logger.info("Best route for packet with dest_ip: " + dest_ip.as_string() + " is: " + route.as_string());
  this->arp(p,route.nexthop,t);
}

ushort ip_protocol::calculateChecksum(ip::Header header) const {

  const char* header_ptr = header.c_str();
  ushort count = header.size();

  long sum = 0;
  char* addr = (char *)header_ptr;

  while( count > 1 )  {
    //  This is the inner loop
    sum += *(ushort*)addr++;
    count -= 2;
  }

  // Add left-over byte, if any
  if( count > 0 )
  sum += *(unsigned char*)addr++;

  // Fold 32-bit sum to 16 bits
  while (sum>>16)
    sum = (sum & 0xffff) + (sum >> 16);

  delete[] header_ptr;
  return ~sum;
}

bool ip_protocol::verifychecksum(ip::Header header) const {
  logger.debug("header checksum: " + std::to_string(header.header_checksum));
  ushort new_checksum = ~calculateChecksum(header);
  logger.debug("calculated checksum: " + std::to_string(new_checksum));
  return new_checksum+header.header_checksum == 0xFFFF;
}

bool ip_protocol::matchesHostIps(IPv4 dest_ip) const {

  for (std::list<IPv4>::const_iterator it = reserved_ips.cbegin(); it != reserved_ips.cend(); ++it) {
    if (dest_ip == *it) {
      return true;
    }
  }
  return false;
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

void ip_protocol::arp(ip::Packet packet, IPv4 nexthop, IPv4 netmask, double t) {
  
  logger.debug("Adding packet to wait ARP for nexthop: " + nexthop.as_string());
  if (arp_waiting_packets.find(nexthop) != arp_waiting_packets.end()) {
    arp_waiting_packets.at(nexthop).push(packet);
  } else {
    std::queue<ip::Packet> q;
    q.push(packet);
    arp_waiting_packets.insert(std::make_pair(nexthop, q));
  }

  link:Control m;
  m.request = link::Ctrl::ARP_QUERY;
  m.ip = nexthop;
  m.interface = this->getInterface(nexthop, netmask);

  Event o = Event(lower_layer_ctrl_out.push(m,t), 3);
  outputs.push(o);
}

void ip_protocol::processLinkControl(link::Control control, double t) {
  std::queue<ip::Packet> packets_to_send
  link::Control m;

  switch(control.request) {
  case link::Ctrl::ARP_READY:
    if (arp_waiting_packets.find(control.ip) == arp_waiting_packets.end()) {
      logger.debug("No packet to send to nexthop " + control.ip.as_string());
      return; 
    }

    packets_to_send = arp_waiting_packets.at(control.ip);
    while (!packets_to_send.empty()) {
      m.request = link::Ctrl::SEND_PACKET;  
      m.packet = packets_to_send.front();
      m.interface = control.interface;
      m.ip = control.ip;
      packets_to_send.pop();

      Event o = Event(lower_layer_ctrl_out.push(m,t), 3);
      outputs.push(o);
    }
    arp_waiting_packets.erase(control.ip);
    break;
  case link::Ctrl::SEND_PACKET_FAILED:
    this->routeIPPacket(control.packet, t); // send again, this sends a new ARP QUERY
    break;
  default:
    logger.debug("Bad link control request.");
  }
}