#include "ip_router_protocol.h"

void ip_router_protocol::dint(double t) {

  last_transition = t;

  if (!arp_ready_packet.empty()) {
    link::Frame f = arp_ready_packet.front();
  	logger.debug("Sending L2 frame for mac: " + f.MAC_destination.as_string());
    output = Event(link_frame_out.push(f,t),2);
    arp_ready_packet.pop();
    next_internal = send_frame_time;
    return;
  }

  if (!arp_in.empty()) {
    ip::arp::Packet p = arp_in.front();
    this->processARPPacket(p,t);
    arp_in.pop();
    next_internal = process_arp_packet_time;
    return;
  }

  if (!link_frame_in.empty()) {
  	logger.debug("Process L2 Frame input.");
    link::Frame f = link_frame_in.front();
    this->processIPPacket(f.payload,t);
    link_frame_in.pop();
    next_internal = process_ip_packet_time;
    return;
  }

  next_internal = infinity;
  output = Event(0,5);
}

/***********************************/
/********* HELPER METHODS **********/
/***********************************/

void ip_router_protocol::processIPPacket(ip::Packet p, double t) {
  ip::Routing_entry route;
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("ip packet: " + dest_ip.as_string());
  // The used algorithm is the microsoft routing process that specifies 7 steps
  // Step 1
  if (!this->verifychecksum(p.header)) {
    // silent discard
    logger.info("Discard packet: checksum verification faild for packet with dest_ip: "
                + dest_ip.as_string());
    output = Event(0,5);
    return;
  }

  // Step 2 ignored, no upper layer in a router. This step was made for routers running in a host

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

  // Step 5-7
  this->routeIPPacket(p,t);
}

bool ip_router_protocol::TTLisZero(ushort ttlp) const {
  ushort ttl = ttlp >> 8;
  logger.debug("TTL: " + std::to_string(ttl));
  return ttl == 0;
}

ushort ip_router_protocol::decreaseTTL(ushort ttlp) const {
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