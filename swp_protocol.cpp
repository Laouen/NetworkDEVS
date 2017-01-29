#include "swp_protocol.h"

void swp_protocol::init(double t, ...) {
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
  logger.info("MAC: " + mac.as_string());

  // reading interface
  interface = (ushort)va_arg(parameters, double);
  logger.info("Interface: " + std::to_string(interface));

  // initializing state
  state.LFS = 0;
  state.LAR = 0;
  state.LFR = 0;

  next_internal = infinity;
  output = Event(0,5);
}

double swp_protocol::ta(double t) {
  if (next_internal == infinity) {
    return this->nexTimeout();
  } else {
    return next_internal;
  }
}

Event swp_protocol::lambda(double) {

  return output;
}

void swp_protocol::dinternal(double t) {

  this->updateTimeouts(t);
  this->updateARPCache(t);

  if (!this->sendWindowsIsFull() && !framesToSend.empty()) {
    link::Frame f = framesToSend.front();
    this->sendSWP(f, t);
    framesToSend.pop();
    next_internal = send_swp_time;
    return;
  }

  if (this->timeoutTriggered()) {
    this->swpTimeout(t);
    next_internal = swp_timeout_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    link::Frame f = lower_layer_data_in.front();
    this->deliverSWP(f, t);
    lower_layer_data_in.pop();
    next_internal = deliver_swp_time;
    return;
  }

  if (!higher_layer_ctrl_in.empty()) {
    link::Control c = higher_layer_ctrl_in.front();
    this->processLinkControl(c), t);
    higher_layer_ctrl_in.pop();
    next_internal = process_link_control_time;
    return;
  }
}

void swp_protocol::dexternal(double t) {
  
  this->updateARPCache(t);
}

void swp_protocol::processLinkControl(link::Control control, double t) {
  link::Frame frame;
  
  if (control.request == link::Ctrl::ARP_QUERY) {
    logger.debug("Bad control: Not ARP_QUERY");
    return;
  }

  switch (control.request) {
  case link::Ctrl::ARP_QUERY:
    for (std::list<link::arp::Entry>::iterator it = ARPTable.begin(); it != ARPTable.end(); ++it) {
      if (it->ip == control.ip) {
        this->sendARPReady(control.ip, t);
        return;
      }
    }
    this->sendArpQuery(control.ip, t);
    break;
  case link::Ctrl::SEND_PACKET:
    for (std::list<link::arp::Entry>::iterator it = ARPTable.begin(); it != ARPTable.end(); ++it) {
      if (it->ip == control.ip) {
        frame = this->wrapInFrame(control.packet, it->mac);
        framesToSend.push(frame);
        return;
      }
    }
    // If MAC is not cached
    this->sendArpQuery(control.ip, t);

    link::Control m;
    m.request = link::Ctrl::SEND_PACKET_FAILED;
    m.packet = control.packet;
    m.interface = interface;
    m.ip = control.ip;
    Event o = Event(higher_layer_ctrl_out.push(m,t), 1);
    outputs.push(o);
    break;
  default:
    logger.debug("Bad control");
    break;
  }
}

void swp_protocol::sendSWP(link::Frame frame, double t) {
  logger.info("sendSWP");
  swp::sendQ_slot<link::Frame> slot;
  swp::SwpHdr hdr;

  hdr.SeqNum = ++state.LFS;
  hdr.Flags = FLAG_HAS_DATA;
  this->storeSwpHdr(hdr, frame.preamble);
  
  slot.msg = frame;
  slot.SeqNum = hdr.SeqNum;
  slot.timeout = SWP_SEND_TIMEOUT;

  state.sendQ.push_back(slot);
  Event o = Event(lower_layer_data_out.push(frame, t), 2);
  outputs.push(o);
  return;
}

void swp_protocol::deliverSWP(link::Frame frame, double t) {
  logger.info("deliverSWP");
  swp::SwpHdr hdr;
  swp::recvQ_slot<link::Frame> slot;

  if (!this->validMAC(frame.MAC_destination)) {
    logger.info("Discarted frame: Invalid MAC");
    return;
  }

  if (!this->verifyCRC(frame)) { // Drop frame with error
    logger.info("Discarted frame: CRC didn't pass");
    return;
  }
  
  this->loadSwpHdr(hdr, frame.preamble);
  if (hdr.Flags & FLAG_ACK_VALID) {  // received an acknowledgment---do SENDER side
    
    if (!this->swpInWindow(state.LAR+1, SWS, hdr.AckNum)) {
      logger.info("Discarted frame: Frame not in SWS windows");
      logger.debug( "fram SeqNum: " + std::to_string(hdr.SeqNum) + 
                    " LAR+1: " + std::to_string(state.LAR+1) + 
                    " SWS: " + std::to_string(SWS));
    }
    
    logger.debug( "waiting ACK frames: " + std::to_string(state.sendQ.size()));
    while (state.LAR != hdr.AckNum) {
      logger.debug( "ACK received for frame with SeqNum: " + 
                    std::to_string(state.sendQ.front().SeqNum));
      state.sendQ.pop_front();
      ++state.LAR;
    };
    logger.debug( "waiting ACK frames: " + std::to_string(state.sendQ.size()));
    return;
  } 

  if (hdr.Flags & FLAG_HAS_DATA) { // received data packet---do RECEIVER side
    
    if (!this->swpInWindow(state.LFR+1, RWS, hdr.SeqNum)) { // drop the message
      logger.info("Discarted frame: Frame not in RWS windows");
      logger.debug( "fram SeqNum: " + std::to_string(hdr.SeqNum) + 
                    " LFR+1: " + std::to_string(state.LFR+1) + 
                    " RWS: " + std::to_string(RWS));
      return;
    }

    slot.msg = frame;
    slot.SeqNum = hdr.SeqNum;
    state.recvQ.push_back(slot);
    state.recvQ.sort(this->recvQ_slotComparator);
    logger.debug("Out of order frames: " + std::to_string(state.recvQ.size()));
    
    while (!state.recvQ.empty() && (state.LFR+1) == state.recvQ.front().SeqNum) {
      logger.info("Deliver frame with SeqNum: " + std::to_string(state.recvQ.front().SeqNum));
      this->processReadyFrame(state.recvQ.front().msg, t);
      ++state.LFR;
      state.recvQ.pop_front();
    }
    
    logger.debug("Out of order frames: " + std::to_string(state.recvQ.size()));
    this->sendAck(frame.MAC_source, t);
  }
}

bool swp_protocol::sendWindowsIsFull() {
  
  return state.sendQ.size() >= SWS;
}

void swp_protocol::storeSwpHdr(const swp::SwpHdr& hdr, unsigned long& preamble) {
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

link::Frame swp_protocol::wrapInFrame(const ip::Packet& packet, MAC dest_mac) {
  link::Frame frame;
  frame.MAC_source = mac;
  frame.MAC_destination = dest_mac;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPayload(packet);
  return frame;
}

link::Frame swp_protocol::wrapInFrame(const ip::arp::Packet& packet) {
  link::Frame frame;
  frame.MAC_source = mac;
  frame.MAC_destination = BROADCAST_MAC_ADDRESS;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPayload(packet);
  return frame;
}

bool swp_protocol::validMAC(MAC MAC_destination) {
  return  MAC_destination == mac || 
          MAC_destination == BROADCAST_MAC_ADDRESS;
}

bool swp_protocol::swpInWindow(swp::SwpSeqno startNum, ushort WS, swp::SwpSeqno seqNum) {
  for (swp::SwpSeqno i=0; i<WS; ++i) {
    swp::SwpSeqno current = startNum+i;
    if (seqNum == current)
      return true;
  }
  return false;
}

void swp_protocol::sendAck(MAC mac_dest, double t) {
  link::Frame frame;
  swp::SwpHdr hdr;

  hdr.AckNum = state.LFR;
  hdr.Flags = FLAG_ACK_VALID;
  this->storeSwpHdr(hdr,frame.preamble);

  frame.MAC_destination = mac_dest;
  frame.MAC_source = mac;
  frame.EtherType = 0; // TODO: check what to put here
  frame.CRC = this->calculateCRC();
  
  Event o = Event(lower_layer_data_out.push(frame, t), 2);
  outputs.push(o);
}

void swp_protocol::loadSwpHdr(swp::SwpHdr& hdr, const unsigned long& preamble) {
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

void swp_protocol::swpTimeout(double t) { 
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = state.sendQ.begin(); it != state.sendQ.end(); ++it) {
    if (it->timeout <= 0) {
      it->timeout = SWP_SEND_TIMEOUT;
      Event o = Event(lower_layer_data_out.push(it->msg, t), 2);
      outputs.push(o);
    }
  }
}

void swp_protocol::updateTimeouts(double t) {
  double elapsed_time = t-last_transition;
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = state.sendQ.begin(); it !=state.sendQ.end(); ++it) {
    if (it->timeout != infinity)
      it->timeout -= elapsed_time;
  }
}

double swp_protocol::nexTimeout() {
  double next = infinity;
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = state.sendQ.begin(); it !=state.sendQ.end(); ++it) {
    if (it->timeout < next) {
      next = it->timeout;
    }
  }
  return next;
}

bool swp_protocol::timeoutTriggered() {
  for(std::list<swp::sendQ_slot<link::Frame>>::iterator it = state.sendQ.begin(); it !=state.sendQ.end(); ++it) {
    if (it->timeout <= 0) {
      return true;
    }
  }
  return false;
}

bool swp_protocol::SeqnoRWSComparator(swp::SwpSeqno a, swp::SwpSeqno b) {
  if (abs(a-b) <= RWS) return a < b;
  return b < a;
}

bool swp_protocol::recvQ_slotComparator(swp::recvQ_slot<link::Frame> a, swp::recvQ_slot<link::Frame> b) {
  return swp_protocol::SeqnoRWSComparator(a.SeqNum, b.SeqNum);
}

ip::Packet swp_protocol::getIpPacket(const link::Frame& frame) {
  ip::Packet packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

link::arp::Packet swp_protocol::getARPPacket(const link::Frame& frame) {
  link::arp::Packet packet;
  memcpy(&packet,frame.payload,sizeof(packet));
  return packet;
}

void swp_protocol::processReadyFrame(link::Frame frame, double t) {
  ip::Packet ip_packet;
  link::arp::Packet arp_packet;
  Event o;
  
  if (frame.preamble & IS_IP_PACKET) {
    ip_packet = this->getIpPacket(frame);
    o = Event(higher_layer_data_out.push(p,t), 0);
    outputs.push(o);
  } else {
    arp_packet = this->getARPPacket(frame);
    this.processARPPacket(frame,t)
  }
}

/****************** ARP methods ***********************/

void swp_protocol::updateARPCache(double t) {
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

void swp_protocol::cacheSourceMAC(MAC source_mac, IPv4 source_ip) {
  link::arp::Entry e;
  e.ip = source_ip;
  e.mac = source_mac;
  e.timeout = ARP_TIMEOUT;
  ARPTable.push_back(e);
}

void swp_protocol::sendARPReady(IPv4 arp_ip, double t) {
  link::Control m;
  m.request = link::Ctrl::ARP_READY;
  m.ip = arp_ip;
  m.interface = interface;
  Event o = Event(higher_layer_ctrl_out.push(m, t), 1);
  outputs.push(o);
}

void swp_protocol::sendArpQuery(IPv4 arp_ip, double t) {
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

  framesToSend.push(this->wrapInFrame(query));
}

void swp_protocol::processARPPacket(link::arp::Packet packet, double t) {
  logger.info("Process ARP packet.");
  if (packet.Operation == 1 && ip == packet.Target_Protocol_Address) { // Query packet

    logger.debug("ARP packet query for ip: " + packet.Target_Protocol_Address.as_string());
    this->cacheSourceMAC(packet.Source_Hardware_Address, packet.Source_Protocol_Address);

    // reply with its own mac
    link::arp::Packet response = packet;
    response.Operation = 0; // response
    response.Target_Hardware_Address = mac;

    framesToSend.push(this->wrapInFrame(packet));
  
  } else if (packet.Operation == 0 && packet.Source_Hardware_Address == mac) { // Response packet

    IPv4 arp_ip = packet.Target_Protocol_Address;
    MAC arp_mac = packet.Target_Hardware_Address;

    logger.debug("ARP packet response for ip: " + arp_ip.as_string());
    logger.debug("ARP packet response mac is: " + arp_mac.as_string());

    this->cacheSourceMAC(arp_mac, arp_ip);
    this->sendARPReady(arp_ip,t);
  }
}

/*********** Method that still left to implement *****************/

unsigned long swp_protocol::calculateCRC() { return 0; }

bool swp_protocol::verifyCRC(link::Frame frame) { return true; }