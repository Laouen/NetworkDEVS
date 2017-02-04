#include "link_protocol.h"

void link_protocol::init(double t, ...) {
  // PowerDEVS parameters

  va_list parameters;
  va_start(parameters, t);

  // Set logger module name
  std::string module_name = va_arg(parameters, char*);
  logger.setModuleName("SWP " + module_name);

  // reading mac
  mac = va_arg(parameters, char*);
  logger.info("MAC: " + mac.as_string());

  // reading ip
  ip = va_arg(parameters, char*);
  logger.info("IP: " + mac.as_string());

  // reading interface
  interface = (ushort)va_arg(parameters, double);
  logger.info("Interface: " + std::to_string(interface));

  // initializing state
  SWPState.reset();

  next_internal = infinity;
  output = Event(0,5);
}

double link_protocol::ta(double t) {
  if (next_internal == infinity) {
    return this->swpNexTimeout();
  } else {
    return next_internal;
  }
}

Event link_protocol::lambda(double) {
  return output;
}

void link_protocol::dinternal(double t) {

  this->swpUpdateTimeouts(t);
  this->arpUpdateCache(t);

  if (SWPState.thereIsFrameToSend()) {
    link::Frame f = SWPState.framesToSend.front();
    this->swpSend(f);
    SWPState.framesToSend.pop();
    next_internal = send_swp_time;
    return;
  }

  if (this->swpTimeoutTriggered()) {
    this->swpTimeout();
    next_internal = swp_timeout_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    link::Frame f = lower_layer_data_in.front();
    this->swpDeliver(f);
    lower_layer_data_in.pop();
    next_internal = deliver_swp_time;
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
  this->arpUpdateCache(t);
}

/***************** SWP **************************/

void link_protocol::swpSend(link::Frame frame) {
  logger.info("swpSend");
  Event msg;
  swp::sendQ_slot<link::Frame> slot;
  swp::SwpHdr hdr;

  hdr.SeqNum = ++SWPState.LFS;
  hdr.Flags = FLAG_HAS_DATA;
  this->swpStoreHdr(hdr, frame.preamble);
  
  slot.msg = frame;
  slot.SeqNum = hdr.SeqNum;
  slot.timeout = SWP_SEND_TIMEOUT;

  SWPState.sendQ.push_back(slot);
  lower_layer_data_out.push(frame, 2);
  return;
}

bool link_protocol::swpTimeoutTriggered() {
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = SWPState.sendQ.begin(); it !=SWPState.sendQ.end(); ++it) {
    if (it->timeout <= 0) {
      return true;
    }
  }
  return false;
}

void link_protocol::swpTimeout() { 
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = SWPState.sendQ.begin(); it != SWPState.sendQ.end(); ++it) {
    if (it->timeout <= 0) {
      it->timeout = SWP_SEND_TIMEOUT;
      lower_layer_data_out.push(it->msg, 2);
    }
  }
}

void link_protocol::swpDeliver(link::Frame frame) {
  logger.info("swpDeliver");
  swp::SwpHdr hdr;
  swp::recvQ_slot<link::Frame> slot;

  if (!this->swpValidMAC(frame.MAC_destination)) {
    logger.info("Discarted frame: Invalid MAC");
    return;
  }

  if (!this->verifyCRC(frame)) { // Drop frame with error
    logger.info("Discarted frame: CRC didn't pass");
    return;
  }
  
  this->swpLoadHdr(hdr, frame.preamble);
  if (hdr.Flags & FLAG_ACK_VALID) {  // received an acknowledgment---do SENDER side
    
    if (!this->swpInWindow(SWPState.LAR+1, SWS, hdr.AckNum)) {
      logger.info("Discarted frame: Frame not in SWS windows");
      logger.debug( "fram SeqNum: " + std::to_string(hdr.SeqNum) + 
                    " LAR+1: " + std::to_string(SWPState.LAR+1) + 
                    " SWS: " + std::to_string(SWS));
    }
    
    logger.debug( "waiting ACK frames: " + std::to_string(SWPState.sendQ.size()));
    while (SWPState.LAR != hdr.AckNum) {
      logger.debug( "ACK received for frame with SeqNum: " + 
                    std::to_string(SWPState.sendQ.front().SeqNum));
      SWPState.sendQ.pop_front();
      ++SWPState.LAR;
    };
    logger.debug( "waiting ACK frames: " + std::to_string(SWPState.sendQ.size()));
    return;
  } 

  if (hdr.Flags & FLAG_HAS_DATA) { // received data packet---do RECEIVER side
    
    if (!this->swpInWindow(SWPState.LFR+1, RWS, hdr.SeqNum)) { // drop the message
      logger.info("Discarted frame: Frame not in RWS windows");
      logger.debug( "fram SeqNum: " + std::to_string(hdr.SeqNum) + 
                    " LFR+1: " + std::to_string(SWPState.LFR+1) + 
                    " RWS: " + std::to_string(RWS));
      return;
    }

    // the sort custom comparator is the one that uses the RWS to know where the seqno
    // has started again and if that case, put smaller seqnos at the end of the list.
    slot.msg = frame;
    slot.SeqNum = hdr.SeqNum;
    SWPState.recvQ.push_back(slot);
    SWPState.recvQ.sort(this->recvQ_slotComparator); 
    logger.debug("Out of order frames: " + std::to_string(SWPState.recvQ.size()));
    
    while (!SWPState.recvQ.empty() && (SWPState.LFR+1) == SWPState.recvQ.front().SeqNum) {
      logger.info("Deliver frame with SeqNum: " + std::to_string(SWPState.recvQ.front().SeqNum));
      this->swpDeliverToUpModules(SWPState.recvQ.front().msg);
      ++SWPState.LFR;
      SWPState.recvQ.pop_front();
    }
    
    logger.debug("Out of order frames: " + std::to_string(SWPState.recvQ.size()));
    this->swpSendAck(frame.MAC_source);
  }
}

void link_protocol::swpDeliverToUpModules(link::Frame frame) {
  ip::Packet ip_packet;
  link::arp::Packet arp_packet;
  Event o;
  
  if (frame.preamble & IS_IP_PACKET) {
    ip_packet = this->getIpPacket(frame);
    higher_layer_data_out.push(ip_packet, 0);
  } else {
    arp_packet = this->getARPPacket(frame);
    this->arpProcessPacket(arp_packet);
  }
}

bool link_protocol::SeqnoRWSComparator(swp::SwpSeqno a, swp::SwpSeqno b) {
  if (abs(a-b) <= RWS) return a < b;
  return b < a;
}

bool link_protocol::recvQ_slotComparator(swp::recvQ_slot<link::Frame> a, swp::recvQ_slot<link::Frame> b) {
  
  return link_protocol::SeqnoRWSComparator(a.SeqNum, b.SeqNum);
}

double link_protocol::swpNexTimeout() {
  double next = infinity;
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = SWPState.sendQ.begin(); it !=SWPState.sendQ.end(); ++it) {
    if (it->timeout < next) {
      next = it->timeout;
    }
  }
  return next;
}

void link_protocol::swpStoreHdr(const swp::SwpHdr& hdr, unsigned long& preamble) {
  preamble = preamble & 0x000FFFFF; // cleaning the bits where swp::SwpHdr will be placed
  unsigned long stored_hdr = 0x00000000; // starting an empty header
  unsigned long curr_val = 0x00000000; // used to correcly set data in their correspondent bits

  // translating the swp header
  // storing SeqNum
  curr_val = hdr.SeqNum;
  stored_hdr = stored_hdr | curr_val;
  stored_hdr = stored_hdr << 4;

  // storing AckNum
  curr_val = hdr.AckNum;
  stored_hdr = stored_hdr | curr_val;
  stored_hdr = stored_hdr << 4;

  // storing Flags
  curr_val = hdr.Flags;
  stored_hdr = stored_hdr | curr_val;
  stored_hdr = stored_hdr << 20;

  preamble = preamble | stored_hdr;
}

void link_protocol::swpSendAck(MAC mac_dest) {
  logger.info("send ACK LFR: " + std::to_string(SWPState.LFR));
  link::Frame frame;
  swp::SwpHdr hdr;

  hdr.AckNum = SWPState.LFR;
  hdr.Flags = FLAG_ACK_VALID;
  this->swpStoreHdr(hdr,frame.preamble);

  frame.MAC_destination = mac_dest;
  frame.MAC_source = mac;
  frame.EtherType = 0; // TODO: check what to put here
  frame.CRC = this->calculateCRC();
  
  lower_layer_data_out.push(frame, 2);
}

bool link_protocol::swpValidMAC(MAC MAC_destination) {
  return  MAC_destination == mac || 
          MAC_destination == BROADCAST_MAC_ADDRESS;
}

bool link_protocol::swpInWindow(swp::SwpSeqno startNum, ushort WS, swp::SwpSeqno seqNum) {
  for (swp::SwpSeqno i=0; i<WS; ++i) {
    swp::SwpSeqno current = startNum+i;
    if (seqNum == current)
      return true;
  }
  return false;
}

void link_protocol::swpLoadHdr(swp::SwpHdr& hdr, const unsigned long& preamble) {
  unsigned long stored_hdr = 0x00000000;

  stored_hdr = 0x00F00000 & preamble;
  stored_hdr = stored_hdr >> 20;
  hdr.Flags = stored_hdr;

  stored_hdr = 0x0F000000 & preamble;
  stored_hdr = stored_hdr >> 24;
  hdr.AckNum = stored_hdr;

  stored_hdr = 0xF0000000 & preamble;
  stored_hdr = stored_hdr >> 28;
  hdr.SeqNum = stored_hdr;
}

void link_protocol::swpUpdateTimeouts(double t) {
  double elapsed_time = t-last_transition;
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = SWPState.sendQ.begin(); it !=SWPState.sendQ.end(); ++it) {
    if (it->timeout != infinity)
      it->timeout -= elapsed_time;
  }
}

/***************** Comunication methods **************************/

link::Frame link_protocol::wrapInFrame(const ip::Packet& packet, MAC dest_mac) {
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

ip::Packet link_protocol::getIpPacket(const link::Frame& frame) {
  ip::Packet packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

link::arp::Packet link_protocol::getARPPacket(const link::Frame& frame) {
  link::arp::Packet packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

void link_protocol::sendControl(link::Ctrl c, link::Control control) {
  logger.info("Send control to ip: " + link::to_string(c));
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
      frame = this->wrapInFrame(control.packet, dest_mac);
      SWPState.sendFrame(frame);
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

void link_protocol::arpProcessPacket(link::arp::Packet packet) {
  logger.debug("Process ARP packet.");
  logger.log(packet.as_string());

  if (packet.Operation == 1 && ip == packet.Target_Protocol_Address) { // Query packet

    logger.debug("ARP packet query owner ip: " + packet.Target_Protocol_Address.as_string());
    this->arpCacheSourceMAC(packet.Source_Hardware_Address, packet.Source_Protocol_Address);

    // reply with its own mac
    link::arp::Packet response = packet;
    response.Operation = 0; // response
    response.Target_Hardware_Address = mac;

    SWPState.sendFrame(this->wrapInFrame(response));
  
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

void link_protocol::arpUpdateCache(double t) {
  double elapsed_time = t-last_transition;
  std::list<link::arp::Entry>::iterator it = ARPTable.begin();

  while (it != ARPTable.end()) {
    if (it->timeout < elapsed_time) {
      ARPTable.erase(it);
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

  SWPState.sendFrame(this->wrapInFrame(query));
}