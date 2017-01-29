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

  if (!this->sendWindowsIsFull() && !higher_layer_data_in.empty()) {
    link::Frame f = higher_layer_data_in.front();
    this->sendSWP(f, t);
    higher_layer_data_in.pop();
    next_internal = send_swp_time;
    return;
  }

  if (!this->sendWindowsIsFull() && !higher_layer_ctrl_in.empty()) {
    ip::arp::Packet p = higher_layer_ctrl_in.front();
    this->sendSWP(this->wrapARPInFrame(p), t);
    higher_layer_ctrl_in.pop();
    next_internal = send_swp_time;
    return;
  }

  if (!readyToDeliver.empty()) {
    link::Frame frame = readyToDeliver.front();
    this->deliverToUpLayer(frame, t);
    readyToDeliver.pop();
    next_internal = deliver_up_layer_time;
  }

  next_internal = infinity;
  output = Event(0,5);
}

void swp_protocol::sendSWP(link::Frame frame, double t) {
  logger.debug("sendSWP");
  swp::sendQ_slot<link::Frame> slot;
  swp::SwpHdr hdr;

  hdr.SeqNum = ++state.LFS;
  hdr.Flags = FLAG_HAS_DATA;
  this->storeSwpHdr(hdr, frame.preamble);
  
  slot.msg = frame;
  slot.SeqNum = hdr.SeqNum;
  slot.timeout = SWP_SEND_TIMEOUT;
  state.sendQ.push_back(slot);
  
  output = Event(lower_layer_data_out.push(frame, t), 2);
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

link::Frame swp_protocol::wrapARPInFrame(const ip::arp::Packet& packet) {
  link::Frame frame;
  frame.MAC_destination = "FF:FF:FF:FF:FF:FF";
  frame.MAC_source = mac;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPayload(packet);
  return frame;
}

void swp_protocol::deliverSWP(link::Frame frame, double t) {
  logger.debug("deliverSWP");
  swp::SwpHdr hdr;
  swp::recvQ_slot<link::Frame> slot;
  output = Event(0,5); // by default there are nothing to send

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
    
    if (this->swpInWindow(state.LAR+1, SWS, hdr.AckNum)) {
      while (state.LAR != hdr.AckNum) {
        state.sendQ.pop_front();
        ++state.LAR;
      };
    }
    return;
  } 

  if (hdr.Flags & FLAG_HAS_DATA) { // received data packet---do RECEIVER side
    
    if (!this->swpInWindow(state.LFR+1, RWS, hdr.SeqNum)) { // drop the message
      logger.debug("Frame not in windows");
      logger.debug(std::to_string(state.LFR+1) + " " + std::to_string(hdr.SeqNum) + " " + std::to_string(RWS));
      return;
    }

    slot.msg = frame;
    slot.SeqNum = hdr.SeqNum;
    state.recvQ.push_back(slot);
    state.recvQ.sort(this->recvQ_slotComparator);
    logger.debug(" Data frame: " + std::to_string(state.recvQ.size()));
    
    while (!state.recvQ.empty() && (state.LFR+1) == state.recvQ.front().SeqNum) {
      readyToDeliver.push(state.recvQ.front().msg);
      ++state.LFR;
      state.recvQ.pop_front();
    }
    
    this->sendAck(frame.MAC_source, t);
  }
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
  output = Event(lower_layer_data_out.push(frame, t), 2);
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
  std::list<swp::sendQ_slot<link::Frame>>::iterator it = state.sendQ.begin();
  while(it != state.sendQ.end() && it->timeout > 0) ++it;
  it->timeout = SWP_SEND_TIMEOUT;
  output = Event(lower_layer_data_out.push(it->msg, t), 2);
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

void swp_protocol::deliverToUpLayer(const link::Frame& frame, double t) {
  if (frame.preamble & IS_IP_PACKET) {
    output = Event(higher_layer_data_out.push(frame, t), 0);
  } else {
    ip::arp::Packet packet;
    memcpy(&packet,frame.payload,sizeof(packet));
    output = Event(higher_layer_ctrl_out.push(packet, t), 1);
  }
}

bool swp_protocol::SeqnoRWSComparator(swp::SwpSeqno a, swp::SwpSeqno b) {
  if (abs(a-b) <= RWS) return a < b;
  return b < a;
}

bool swp_protocol::recvQ_slotComparator(swp::recvQ_slot<link::Frame> a, swp::recvQ_slot<link::Frame> b) {
  return swp_protocol::SeqnoRWSComparator(a.SeqNum, b.SeqNum);
}

/*********** Method that still left to implement *****************/

unsigned long swp_protocol::calculateCRC() { return 0; }

bool swp_protocol::verifyCRC(link::Frame frame) { return true; }