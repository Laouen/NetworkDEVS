#include "ip_protocol.h"

// Note: The used algorithm is the microsoft ip routing process at a sender

void ip_protocol::init(double t,...) {

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
    host_ips.push_back(va_arg(parameters,char*));
  }

  logger.info("ips:");
  for(std::list<IPv4>::iterator i = host_ips.begin(); i != host_ips.end(); ++i){
    logger.info(i->as_string());
  }

  // building routing table
  const char* file_path = va_arg(parameters,char*);
  Parser<ip::Routing_entry> parser_rt(file_path);
  if (parser_rt.file_open()) {
    while(true) {
      try {
        routing_table.push_back(parser_rt.next_input());
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
  if (parser_ft.file_open()) {
    while(true) {
      try {
        forwarding_table.push_back(parser_ft.next_input());
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

void ip_protocol::routeIPDatagram(ip::Datagram p) {
  ip::Routing_entry route;
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("sending ip packet: " + dest_ip.as_string());
  
  if (!this->getBestRoute(dest_ip, route)) {
    logger.info("Discard packet: No route for packet with dest_ip: " + dest_ip.as_string());
    ip::Control m = ip::Control(ip::Ctrl::ROUTING_ERROR);
    higher_layer_ctrl_out.push(m, 1);
    return; 
  }

  // Step 6-7
  logger.info("Best route for packet with dest_ip: " + dest_ip.as_string() + " is: " + route.as_string());
  this->arp(p,route.nexthop);
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

  for (std::list<IPv4>::const_iterator it = host_ips.cbegin(); it != host_ips.cend(); ++it) {
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

void ip_protocol::arp(ip::Datagram packet, IPv4 nexthop) {
  
  logger.debug("Adding packet to wait ARP for nexthop: " + nexthop.as_string());
  if (arp_waiting_packets.find(nexthop) != arp_waiting_packets.end()) {
    arp_waiting_packets.at(nexthop).push(packet);
  } else {
    std::queue<ip::Datagram> q;
    q.push(packet);
    arp_waiting_packets.insert(std::make_pair(nexthop, q));
  }

  link::Control m;
  m.request = link::Ctrl::ARP_QUERY;
  m.ip = nexthop;
  bool interfaceFound = this->getInterface(nexthop, m.interface);

  if (!interfaceFound) {
    logger.error("interface not found.");
    //TODO: report to up layer.
    return;
  }

  logger.info("ARP query throw interface: NET " + std::to_string(m.interface));
  lower_layer_ctrl_out.push(m,3);
}

void ip_protocol::processLinkControl(link::Control control) {
  std::queue<ip::Datagram> packets_to_send;
  link::Control m;

  switch(control.request) {
  case link::Ctrl::ARP_READY:
    logger.info("link::Ctrl::ARP_READY");
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

      lower_layer_ctrl_out.push(m,3);
    }
    arp_waiting_packets.erase(control.ip);
    break;
  case link::Ctrl::SEND_PACKET_FAILED:
    logger.info("link::Ctrl::SEND_PACKET_FAILED");
    this->routeIPDatagram(control.packet); // send again, this sends a new ARP QUERY
    break;
  default:
    logger.error("Bad link control request.");
  }
}

bool ip_protocol::getInterface(IPv4 nexthop, ushort& interface) const {
  for (std::list<ip::Forwarding_entry>::const_iterator it = forwarding_table.begin(); it != forwarding_table.end(); ++it) {
    if (it->sameSubnet(nexthop)) {
      interface = it->interface;
      return true;
    }
  }
  return false;
}