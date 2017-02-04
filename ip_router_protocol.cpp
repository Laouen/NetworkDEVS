#include "ip_router_protocol.h"

void ip_router_protocol::dinternal(double t) {

  if (!lower_layer_ctrl_in.empty()) {
    link::Control c = lower_layer_ctrl_in.front();
    this->processLinkControl(c);
    lower_layer_ctrl_in.pop();
    next_internal = process_link_control_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
  	logger.debug("Process L2 Frame input.");
    ip::Packet p = lower_layer_data_in.front();
    this->processIPPacket(p);
    lower_layer_data_in.pop();
    next_internal = process_ip_packet_time;
    return;
  }

  next_internal = infinity;
  output = Event(0,5);
}

/***********************************/
/********* HELPER METHODS **********/
/***********************************/

void ip_router_protocol::processIPPacket(ip::Packet p) {
  ip::Routing_entry route;
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("ip packet: " + dest_ip.as_string());
  // The used algorithm is the microsoft routing process that specifies 7 steps
  // Step 1
  if (!this->verifychecksum(p.header)) {
    // silent discard
    logger.info("Discard packet: checksum verification faild for packet with dest_ip: "
                + dest_ip.as_string());
    return;
  }

  // Step 2 ignored, no upper layer in a router. This step was made for 
  // routers running in a host

  // Step 3  
  if (this->TTLisZero(p.header.ttlp)) {
    // silent discard. Some IP implementations send 
    // ICMP destination Unreachable-Network message to sender
    logger.info("Discard packet: TTL zero for packet with dest_ip: " 
                + dest_ip.as_string());
    return; 
  }

  // Step 4  
  p.header.ttlp = this->decreaseTTL(p.header.ttlp);
  // Recalculates checksum because the TTL field has being updated
  p.header.header_checksum = this->calculateChecksum(p.header);

  // Step 5-7
  this->routeIPPacket(p);
}

bool ip_router_protocol::TTLisZero(ushort ttlp) const {
  ushort ttl = ttlp >> 8;
  logger.info("TTL: " + std::to_string(ttl));
  return ttl == 0;
}

ushort ip_router_protocol::decreaseTTL(ushort ttlp) const {
  ushort ttl = ttlp >> 8;
  logger.debug("Before decrease TTL: " + std::to_string(ttl));
  
  --ttl;
  ttl = ttl << 8;
  ttlp = ttlp & 0x00FF;
  ttlp = ttl | ttlp;
  
  logger.debug("After decrease TTL: " + std::to_string(ttl));
  return ttlp;
}