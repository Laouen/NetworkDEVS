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
  slot =  &state.sendQ[state.hdr.SeqNum % swp::SWS];
  state.hdr.SeqNum = ++state.LFS;
  state.hdr.Flags = state.hdr.Flags | swp::FLAG_HAS_DATA;
  this->store_swp_hdr(state.hdr, frame.preamble);
  slot->msg = frame;
  slot->timeout = swp::SWP_SEND_TIMEOUT;
  output = Event(lower_layer_data_out.push(frame, t), 2);
}

// TODO: unit test this method
void swp_protocol::store_swp_hdr(const swp::SwpHdr& hdr, unsigned long preamble&) {
  preamble = preamble & 0x000FFFFF; // cleaning the bits where swp::SwpHdr will be placed
  unsigned long stored_hdr = 0x00000000; // starting an empty header
  unsigned long curr_val = 0x00000000; // used to correcly set data in their correspondent bits

  // translating the swp header
  // storing SeqNum
  curr_val = hdr.SeqNum;
  stored_hdr = stored_hdr & curr_val;
  stored_hdr<<8;

  // storing AckNum
  curr_val = hdr.AckNum;
  stored_hdr = stored_hdr & curr_val;
  stored_hdr<<8;

  // storing Flags
  curr_val = hdr.Flags;
  stored_hdr = stored_hdr & curr_val;
  stored_hdr<<8;

  stored_hdr<<40;
  preamble = preamble | stored_hdr;
}

/*********** Method that still left to implement *****************/

link::Frame swp_protocol::wrapInFrame(ip::arp::Packet packet) {
  
}

void swp_protocol::deliverSWP(link::Frame frame, double t) {}

void swp_protocol::swpTimeout(double t) {}

/*********** Method that still left to implement *****************/

void swp_protocol::updateTimeouts(double t) {
  double elapsed_time = t-last_transition;
  for(int i=0; i<swp::SWS; ++i) {
    state.sendQ[i].timeout -= elapsed_time;
  }
}

double swp_protocol::nexTimeout() {
  double next = infinity;
  for(int i=0; i < swp::SWS; ++i) {
    if (state.sendQ[i].timeout < next) {
      next = state.sendQ[i].timeout;
    }
  }
  return next;
}

bool swp_protocol::timeoutTriggered() {
  for(int i=0; i < swp::SWS; ++i) {
    if (state.sendQ[i].timeout <= 0) {
      return true;
    }
  }
  return false;
}