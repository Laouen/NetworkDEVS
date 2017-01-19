#include "ip_host_protocol.h"

void ip_host_protocol::dinternal(double t) {

  if (!arp_ready_packet.empty()) {
    link::Frame f = arp_ready_packet.front();
    logger.debug("Sending L2 frame for mac: " + f.MAC_destination.as_string());
    output = Event(lower_layer_data_out.push(f,t),2);
    arp_ready_packet.pop();
    next_internal = send_frame_time;
    return;
  }

  if (!lower_layer_ctrl_in.empty()) {
    ip::arp::Packet p = lower_layer_ctrl_in.front();
    this->processARPPacket(p,t);
    lower_layer_ctrl_in.pop();
    next_internal = process_arp_packet_time;
    return;
  }

  if (!higher_layer_data_in.empty()) {
    udp::Datagram d = higher_layer_data_in.front();
    this->processUDPDatagram(d,t);
    higher_layer_data_in.pop();
    next_internal = process_udp_datagram_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    logger.debug("Process L2 Frame input.");
    link::Frame f = lower_layer_data_in.front();
    this->processIPPacket(f.payload,t);
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

void ip_host_protocol::processIPPacket(ip::Packet p, double t) {
  ip::Routing_entry route;
  IPv4 dest_ip = p.header.dest_ip; 

  logger.debug("processing arrived ip packet: " + dest_ip.as_string());

  if (!this->verifychecksum(p.header)) {
    // silent discard
    logger.info("Discard packet: checksum verification faild for packet with dest_ip: "
                + dest_ip.as_string());
    output = Event(0,5);
    return;
  }

  if (!this->matchesHostIps(dest_ip)) {
    // silent discard
    logger.info("Discard packet: host ip does not match for packet with dest_ip: "
                + dest_ip.as_string());
    output = Event(0,5);
    return;
  }

  // Delivering datagram to the next top layer
  output = Event(higher_layer_data_out.push(p.data,t),0);
  return;
}

void ip_host_protocol::processUDPDatagram(udp::Datagram d, double t) {
  ip::Packet p;
  p.data = d;
  // version 0100 (IPv4) IHL 0101 (5 no option field)
  // DSCP 0000 (currently not used) ECN 0000 (currently not used)
  p.header.vide = 0x4500;
  p.header.total_length = sizeof(ip::Header) + d.psd_header.udp_lenght;
  // set ttl in hexa FF (max posible TTL) 
  // set protocol in hexa 11 dec 17 is the protocol number of UDP
  p.header.ttlp = 0xFF11;
  p.header.src_ip = d.psd_header.src_ip;
  p.header.dest_ip = d.psd_header.dest_ip;
  p.header.header_checksum = this->calculateChecksum(p.header);

  this->routeIPPacket(p,t);
}