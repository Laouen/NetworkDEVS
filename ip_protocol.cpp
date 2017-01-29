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

  // reading ip
  ip = va_arg(parameters,char*);
  logger.info("IP: " + ip.as_string());

  // reading initial reserved ips
  int ip_amount = (int)va_arg(parameters,double);
  logger.info("Reserved ip amount: " + std::to_string(ip_amount));
  for(int i=0;i<ip_amount;++i){
    reserved_ips.push_back(va_arg(parameters,char*));
  }

  logger.info("Reserved ips:");
  for(std::list<IPv4>::iterator i = reserved_ips.begin(); i != reserved_ips.end(); ++i){
    logger.info(i->as_string());
  }

  // reading MAC address
  mac = va_arg(parameters,char*);
  logger.info("MAC: " + mac.as_string());  

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

ip::Packet ip_protocol::getIpPacket(const link::Frame& frame) {
  ip::Packet packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

void ip_protocol::routeIPPacket(ip::Packet p, double t) {
  ip::Routing_entry route;
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("sending ip packet: " + dest_ip.as_string());
  
  if (!this->getBestRoute(dest_ip, route)) {
    // silent discard
    logger.info("Discard packet: No route for packet with dest_ip: " 
                + dest_ip.as_string());
    ip::Control m = ip::Control(ip::Ctrl::ROUTING_ERROR);
    output = Event(higher_layer_ctrl_out.push(m,t), 1);
    return; 
  }

  // Step 6-7
  logger.info("Best route for packet with dest_ip: " 
              + dest_ip.as_string() + " is: " + route.as_string());
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
  
  // True if matches the host ip or one of the reserved ip like 127.0.0.1
  if (dest_ip == ip) return true;
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

/***********************************************************************/
/*************************** ARP METHODS *******************************/
/***********************************************************************/

void ip_protocol::sendPacket(ip::Packet packet, MAC nexthop_mac) {

  link::Frame frame;
  frame.MAC_destination = nexthop_mac;
  frame.MAC_source = mac;
  frame.EtherType = 0; // TODO: check what to put here, sizeof(packet)
  frame.setPayload(packet);
  arp_ready_packet.push(frame);
}

void ip_protocol::arp(ip::Packet packet, IPv4 nexthop, double t) {

  
  if (forwarding_table.find(nexthop) != forwarding_table.end()) {
    ip::Forwarding_entry e = forwarding_table[nexthop];
    logger.info("MAC address found in cache: " + e.MAC_address.as_string());
    this->sendPacket(packet, e.MAC_address);
    return;
  }

  logger.info("MAC address didn't find.");
  // If MAC address wasn't found, then a ARP query is sent to all the hosts/routers directly attached
  if (arp_waiting_packets.find(nexthop) != arp_waiting_packets.end()) {
    logger.debug("Adding packet to wait ARP for nexthop: " + nexthop.as_string());
    arp_waiting_packets[nexthop].push(packet);
  } else {
    logger.debug("Adding packet to wait ARP for nexthop: " + nexthop.as_string());
    std::queue<ip::Packet> q;
    q.push(packet);
    arp_waiting_packets.insert(std::make_pair(nexthop, q));
  }

  this->sendArpQuery(nexthop,t);
}

void ip_protocol::sendArpQuery(IPv4 nexthop, double t) {
  logger.info("Sending ARP query for ip: " + nexthop.as_string());
  
  ip::arp::Packet query;
  query.Hardware_type = 1;
  query.Protocol_type = 0x0800;
  query.HLen = 48;
  query.PLen = 32;
  query.Operation = 1; // query
  query.Source_Hardware_Address = mac;
  query.Source_Protocol_Address = ip;
  query.Target_Hardware_Address = "0:0:0:0:0:0"; // In a query this field is ignored
  query.Target_Protocol_Address = nexthop;

  output = Event(lower_layer_ctrl_out.push(query,t),3);
}

void ip_protocol::cacheSourceMAC(MAC source_mac, IPv4 source_ip) {
  ip::Forwarding_entry e;
  e.nexthop = source_ip;
  e.MAC_address = source_mac;
  forwarding_table[e.nexthop] = e;
}

void ip_protocol::processARPPacket(ip::arp::Packet packet, double t) {
  logger.info("Process ARP packet.");
  if (packet.Operation == 1 && ip == packet.Target_Protocol_Address) { // Query packet

    logger.debug("ARP packet query for ip: " + packet.Target_Protocol_Address.as_string());
    this->cacheSourceMAC(packet.Source_Hardware_Address, packet.Source_Protocol_Address);

    // reply with its own mac
    ip::arp::Packet response = packet;
    response.Operation = 0; // response
    response.Target_Hardware_Address = mac;
    output = Event(lower_layer_ctrl_out.push(response,t),3);
  
  } else if (packet.Operation == 0 && packet.Source_Hardware_Address == mac) { // Response packet

    IPv4 nexthop_ip = packet.Target_Protocol_Address;
    MAC nexthop_mac = packet.Target_Hardware_Address;

    logger.debug("ARP packet response for nexthop ip: " + nexthop_ip.as_string());
    logger.debug("ARP packet response mac address is: " + nexthop_mac.as_string());

    this->cacheSourceMAC(nexthop_mac, nexthop_ip);

    // send the packet that were waiting for this nexthop MAC address
    if (arp_waiting_packets.find(nexthop_ip) != arp_waiting_packets.end()) {
      while(!arp_waiting_packets[nexthop_ip].empty()) {
        logger.debug("Sending ready packet to mac address: " + nexthop_mac.as_string());
        this->sendPacket(arp_waiting_packets[nexthop_ip].front(), nexthop_mac);
        arp_waiting_packets[nexthop_ip].pop();
      }
    }
  }
}