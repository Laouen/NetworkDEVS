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
    link::Frame f lower_layer_data_in.front();
    this->deliverSWP(f, t);
    lower_layer_data_in.pop();
    next_internal = deliver_swp_time;
    return;
  }

  if (state.sendWindowEmptyPlaces > 0 && !higher_layer_data_in.empty()) {
    link::Frame f = higher_layer_data_in.front();
    this->sendSWP(f, t);
    higher_layer_data_in.pop();
    next_internal = send_swp_time;
    return;
  }

  if (state.sendWindowEmptyPlaces > 0 && !higher_layer_ctrl_in.empty()) {
    ip::arp::Packet p = higher_layer_ctrl_in.front();
    this->sendSWP(this->wrapInFrame(p), t);
    higher_layer_ctrl_in.pop();
    next_internal = send_swp_time;
    return;
  }

  next_internal = infinity;
  output = Event(0,5);
}

void swp_protocol::sendSWP(link::Frame frame, double t) {
  struct sendQ_slot* slot;

  state.sendWindowEmptyPlaces--;
  state.hdr.SeqNum = ++state.LFS;
  state.hdr.Flags = swp::FLAG_HAS_DATA;
  this->store_swp_hdr(state.hdr, frame.preamble);
  slot = &state.sendQ[state.hdr.SeqNum % SWS]; //TODO: Check this;
  slot->msg = frame;
  slot->timeout = swp::SWP_SEND_TIMEOUT;
  output = Event(lower_layer_data_out.push(frame, t), 2);
}

void swp_protocol::store_swp_hdr(const swp::SwpHdr& hdr, unsigned long& preamble) {
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

link::Frame swp_protocol::wrapInFrame(const ip::arp::Packet& packet) {
  link::Frame frame;
  frame.MAC_destination = "FF:FF:FF:FF:FF:FF";
  frame.MAC_source = mac;
  frame.EtherType = 0; // TODO: check what to put here;
  frame.setPyload(packet);
  return frame;
}

void swp_protocol::deliverSWP(link::Frame frame, double t) {

  SwpHdr hdr;
  this->load_swp_hdr(hdr, frame.preamble);
  if (hdr->Flags & FLAG_ACK_VALID) { 
    /* received an acknowledgment---do SENDER side */
    struct sendQ_slot *slot;
    if (this->swpInWindow(hdr.AckNum)) {
      do {
        slot = &state->sendQ[++state->LAR % SWS]; // check this, same problem as in the sendSWP method
        slot->timeout = infinity; // this is a event cancelation
        slot->msg = link::Frame(); // this is a removed msg
        state.sendWindowEmptyPlaces++; // this is a semSignal
      } while (state->LAR != hdr.AckNum);
    }
    return;
  }
  
  if (hdr.Flags & FLAG_HAS_DATA) {
    struct recvQ_slot *slot;
    /* received data packet---do RECEIVER side */
    slot = &state->recvQ[hdr.SeqNum % RWS];
    if (!this->swpInWindow(hdr.SeqNum)) { /* drop the message */
      return;
    }

    slot->msg = frame;
    slot->received = true;
    if (hdr.SeqNum == state->NFE) {
      while (slot->received) {
        readyToDeliver.push(slot->msg);
        slot->msg = link::Frame(); // this is a removed msg
        slot->received = false;
        slot = &state->recvQ[++state->NFE % RWS];
      }

      this->sendAck(hdr, frame.MAC_source, t);
    }
  }
  return;
}

bool swp_protocol::swpInWindow(swp::SwpSeqno seqNum) {
  // NFE = LFR+1;
  // NFE+RWS-1 = LAF
  return !(seqNum < state->NFE || seqNum > (state->NFE+RWS-1));
}

void swp_protocol::sendAck(swp::SwpHdr sh, MAC mac_dest, double t) {
  link::Frame frame;
  sh.AckNum = state->NFE-1;
  sh.Flags = swp::FLAG_ACK_VALID;
  this->store_swp_hdr(sh,frame.preamble);
  frame.MAC_destination mac_dest;
  frame.MAC_source = mac;
  frame.EtherType = 0; // TODO: check what to put here
  frame.CRC = this->calculateCRC();
  // frame.payload TODO: Put an empty payload
  output = Event(lower_layer_data_out.push(frame, t), 2);
}

/*********** Method that still left to implement *****************/

void swp_protocol::load_swp_hdr(swp::SwpHdr& hdr, const unsigned long& preamble) {}

void swp_protocol::swpTimeout(double t) {}

unsigned long swp_protocol::calculateCRC() {
  return 0;
}

/*********** Method that still left to implement *****************/

void swp_protocol::updateTimeouts(double t) {
  double elapsed_time = t-last_transition;
  for(int i=0; i<SWS; ++i) {
    if (state.sendQ[i].timeout != infinity)
      state.sendQ[i].timeout -= elapsed_time;
  }
}

double swp_protocol::nexTimeout() {
  double next = infinity;
  for(int i=0; i < SWS; ++i) {
    if (state.sendQ[i].timeout < next) {
      next = state.sendQ[i].timeout;
    }
  }
  return next;
}

bool swp_protocol::timeoutTriggered() {
  for(int i=0; i < SWS; ++i) {
    if (state.sendQ[i].timeout <= 0) {
      return true;
    }
  }
  return false;
}