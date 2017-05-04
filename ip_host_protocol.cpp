#include "ip_host_protocol.h"

void ip_host_protocol::dinternal(double t) {

  if (!lower_layer_ctrl_in.empty()) {
    link::Control c = lower_layer_ctrl_in.front();
    this->processLinkControl(c);
    lower_layer_ctrl_in.pop();
    next_internal = process_link_control_time;
    return;
  }

  if (!higher_layer_data_in.empty()) {
    udp::Segment d = higher_layer_data_in.front();
    this->processUDPSegment(d);
    higher_layer_data_in.pop();
    next_internal = process_udp_segment_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    logger.debug("Process ip Datagram input.");
    ip::Datagram p = lower_layer_data_in.front();
    this->processIPDatagram(p);
    lower_layer_data_in.pop();
    next_internal = process_ip_datagram_time;
    return;
  }
}

/***********************************/
/********* HELPER METHODS **********/
/***********************************/

void ip_host_protocol::processIPDatagram(ip::Datagram p) {
  IPv4 dest_ip = p.header.dest_ip; 
  Event o;

  logger.info("Processing arrived ip packet: " + dest_ip.as_string());

  if (!this->verifychecksum(p.header)) { // silent discard
    logger.info("Discard packet: checksum verification faild for packet with dest_ip: " + dest_ip.as_string());
    return;
  }

  if (!this->matchesHostIps(dest_ip)) { // Silent discard
    logger.info("Discard packet: host ip does not match for packet with dest_ip: " + dest_ip.as_string());
    return;
  }

  // Delivering segment to the next top layer
  higher_layer_data_out.push(p.data);
  return;
}

void ip_host_protocol::processUDPSegment(udp::Segment d) {
  ip::Datagram p;
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

  this->routeIPDatagram(p);
}