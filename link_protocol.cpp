#include "link_protocol.h"

void link_protocol::init(double t, ...) {
  // PowerDEVS parameters

  va_list parameters;
  va_start(parameters, t);

  // Set logger module name
  std::string module_name = va_arg(parameters, char*);
  logger.setModuleName("Link " + module_name);

  // reading mac
  mac = va_arg(parameters, char*);
  logger.info("MAC: " + mac.as_string());

  // reading ip
  ip = va_arg(parameters, char*);
  logger.info("IP: " + ip.as_string());

  // reading interface
  interface = (ushort)va_arg(parameters, double);
  logger.info("Interface: " + std::to_string(interface));

  swp.init(mac, module_name);

  next_internal = infinity;
  output = Event(0,5);
}

double link_protocol::ta(double t) {
  return std::min(next_internal, swp.nexTimeout());
}

Event link_protocol::lambda(double) {
  return output;
}

void link_protocol::dinternal(double t) {

  swp.updateTimeouts(t-last_transition);
  this->arpUpdateCache(t-last_transition);

  if (!swp.to_send_frames.empty()) {
    this->sendFrames();
    next_internal = send_frame_time;
    return;
  }

  if (swp.thereIsFrameToSend()) {
    swp.send();
    next_internal = swp.send_time;
    return;
  }

  if (!swp.accepted_frames.empty()) {
    this->processAcceptedFrames();
    next_internal = deliver_swp_time;
    return;
  }

  if (swp.timeoutTriggered()) {
    swp.timeout();
    next_internal = swp_timeout_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    link::Frame f = lower_layer_data_in.front();
    this->processFrame(f);
    lower_layer_data_in.pop();
    next_internal = process_frame_time;
    return;
  }

  if (!higher_layer_ctrl_in.empty()) {
    link::Control c = higher_layer_ctrl_in.front();
    this->arpProcessLinkControl(c);
    higher_layer_ctrl_in.pop();
    next_internal = process_link_control_time;
    return;
  }
}

void link_protocol::dexternal(double t) {
  swp.updateTimeouts(t-last_transition);
  this->arpUpdateCache(t-last_transition);
}

/***************** Comunication methods **************************/

void link_protocol::processFrame(link::Frame frame) {
  if (!this->validMAC(frame.MAC_destination)) { // Drop frame not address to the correct mac
    logger.info("Discarted frame: Invalid MAC");
    return;
  }

  if (!this->verifyCRC(frame)) { // Drop frame with error
    logger.info("Discarted frame: CRC didn't pass");
    return;
  }

  swp.processFrame(frame);
}

void link_protocol::processAcceptedFrames() {
  logger.debug("link_protocol::processAcceptedFrames");
  link::Frame frame;
  ip::Datagram datagram;

  while (!swp.accepted_frames.empty()) {
    frame = swp.accepted_frames.front();
    if (frame.preamble & IS_ARP_PACKET) {
      this->arpProcessPacket(frame);
    } else {
      logger.debug("send data to up layer");
      memcpy(&datagram,&frame.payload[0],sizeof(datagram));
      higher_layer_data_out.push(datagram,0);
    }
    swp.accepted_frames.pop();
  }
}

void link_protocol::sendFrames() {
  logger.debug("link_protocol::sendFrames");
  link::Frame frame;

  while(!swp.to_send_frames.empty()) {
    frame = swp.to_send_frames.front();
    frame.CRC = this->calculateCRC();
    lower_layer_data_out.push(frame,2);
    swp.to_send_frames.pop();
  }
}

bool link_protocol::validMAC(MAC MAC_destination) {
  return  MAC_destination == mac || 
          MAC_destination == BROADCAST_MAC_ADDRESS;
}

link::Frame link_protocol::wrapInFrame(const ip::Datagram& packet, MAC dest_mac) {
  link::Frame frame;
  frame.MAC_source = mac;
  frame.MAC_destination = dest_mac;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPayload(packet);
  frame.CRC = this->calculateCRC();
  return frame;
}

link::Frame link_protocol::wrapInFrame(const link::arp::Packet& packet) {
  link::Frame frame;
  frame.MAC_source = mac;
  frame.MAC_destination = BROADCAST_MAC_ADDRESS;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPayload(packet);
  frame.CRC = this->calculateCRC();
  return frame;
}

link::Frame link_protocol::wrapInFrame(const link::arp::Packet& packet, MAC dest_mac) {
  link::Frame frame;
  frame.MAC_source = mac;
  frame.MAC_destination = dest_mac;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPayload(packet);
  frame.CRC = this->calculateCRC();
  return frame;
}

ip::Datagram link_protocol::getIpDatagram(const link::Frame& frame) {
  ip::Datagram packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

link::arp::Packet link_protocol::getARPPacket(const link::Frame& frame) {
  link::arp::Packet packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

void link_protocol::sendControl(link::Ctrl c, link::Control control) {
  logger.info("Send control " + link::to_string(c));
  control.request = c;
  control.interface = interface;
  higher_layer_ctrl_out.push(control, 1);
}

/*********** Unimplemented Comunication Methods *****************/

unsigned long link_protocol::calculateCRC() { return 0; }

bool link_protocol::verifyCRC(link::Frame frame) { return true; }

/****************** ARP methods ***********************/

void link_protocol::arpProcessLinkControl(link::Control control) {
  MAC dest_mac;
  link::Frame frame;
  link::Control m;
  Event o;
  logger.debug("process link control");

  switch (control.request) {
  case link::Ctrl::ARP_QUERY:
    logger.info("ARP_QUERY");
    if (this->arpCachedMAC(control.ip)) {
      this->sendControl(link::Ctrl::ARP_READY, control);
      break;
    }
    this->arpSendQuery(control.ip);
    break;
  case link::Ctrl::SEND_PACKET:
    logger.info("SEND_PACKET");
    if (this->arpCachedMAC(control.ip)) {
      dest_mac = this->arpGetMAC(control.ip);
      logger.debug("Send to MAC: " + dest_mac.as_string());
      frame = this->wrapInFrame(control.packet, dest_mac);
      swp.sendFrame(frame);
      break;
    }
    // If MAC is not cached
    this->arpSendQuery(control.ip);
    this->sendControl(link::Ctrl::SEND_PACKET_FAILED, control);
    break;
  default:
    logger.error("default case: Bad control");
    break;
  }
}

void link_protocol::arpProcessPacket(link::Frame frame) {
  logger.debug("Process ARP packet.");
  
  link::arp::Packet packet = this->getARPPacket(frame);

  if (packet.Operation == 1 && ip == packet.Target_Protocol_Address) { // Query packet

    logger.debug("ARP packet query owner ip: " + packet.Target_Protocol_Address.as_string());
    this->arpCacheSourceMAC(packet.Source_Hardware_Address, packet.Source_Protocol_Address);

    // reply with its own mac
    link::arp::Packet response = packet;
    response.Operation = 0; // response
    response.Target_Hardware_Address = mac;

    swp.sendFrame(this->wrapInFrame(response,frame.MAC_source));
  
  } else if (packet.Operation == 0 && packet.Source_Hardware_Address == mac) { // Response packet

    IPv4 arp_ip = packet.Target_Protocol_Address;
    MAC arp_mac = packet.Target_Hardware_Address;

    logger.debug("ARP packet response for ip: " + arp_ip.as_string());
    logger.debug("ARP packet response mac is: " + arp_mac.as_string());

    this->arpCacheSourceMAC(arp_mac, arp_ip);
    link::Control control;
    control.ip = arp_ip;
    this->sendControl(link::Ctrl::ARP_READY, control);
  }
}

bool link_protocol::arpCachedMAC(IPv4 dest_ip) {
  std::list<link::arp::Entry>::iterator it;

  for (it = ARPTable.begin(); it != ARPTable.end(); ++it) {
    if (it->ip == dest_ip) {
      logger.info("arp ip " + dest_ip.as_string() + " is in cache.");
      return true;
    }
  }
  logger.info("arp ip " + dest_ip.as_string() + " not cached.");
  return false;
}

MAC link_protocol::arpGetMAC(IPv4 dest_ip) {
  std::list<link::arp::Entry>::iterator it;

  for (it = ARPTable.begin(); it != ARPTable.end(); ++it) {
    if (it->ip == dest_ip) 
      return it->mac;
  }
}

void link_protocol::arpUpdateCache(double elapsed_time) {
  std::list<link::arp::Entry>::iterator it = ARPTable.begin();
  while (it != ARPTable.end()) {
    if (it->timeout < elapsed_time) {
      it = ARPTable.erase(it);
    } else {
      it->timeout -= elapsed_time;
      ++it;
    }
  }
}

void link_protocol::arpCacheSourceMAC(MAC source_mac, IPv4 source_ip) {
  link::arp::Entry e;
  e.ip = source_ip;
  e.mac = source_mac;
  e.timeout = ARP_TIMEOUT;
  ARPTable.push_back(e);
}

void link_protocol::arpSendQuery(IPv4 arp_ip) {
  logger.info("Sending ARP query for ip: " + arp_ip.as_string());
  
  link::arp::Packet query;
  query.Hardware_type = 1;
  query.Protocol_type = 0x0800;
  query.HLen = sizeof(MAC);
  query.PLen = sizeof(IPv4);
  query.Operation = 1; // query
  query.Source_Hardware_Address = mac;
  query.Source_Protocol_Address = ip;
  query.Target_Hardware_Address = BROADCAST_MAC_ADDRESS; // In a query this field is ignored
  query.Target_Protocol_Address = arp_ip;

  swp.sendFrame(this->wrapInFrame(query));
}