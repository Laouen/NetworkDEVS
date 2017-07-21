#include "swp_protocol.h"

void swp_protocol::init(MAC other_mac, std::string module_name) {
  mac = other_mac;
  logger.setModuleName("SWP " + module_name);
}

swp_protocol::swp_protocol() {}

bool swp_protocol::thereIsFrameToSend() { 

  return swp_state.thereIsFrameToSend();
}

void swp_protocol::send() {
  logger.debug("swp_protocol::send");

  link::Frame frame = swp_state.framesToSend.front();

  Event msg;
  swp::sendQ_slot<link::Frame> slot;
  swp::Hdr hdr;

  hdr.SeqNum = ++swp_state.LFS;
  hdr.Flags = FLAG_HAS_DATA;
  this->storeHdr(hdr, frame.preamble);
  
  slot.msg = frame;
  slot.SeqNum = hdr.SeqNum;
  slot.timeout = SWP_SEND_TIMEOUT;

  swp_state.sendQ.push_back(slot);
  to_send_frames.push(frame);

  swp_state.framesToSend.pop();
  return;
}

void swp_protocol::sendFrame(link::Frame frame) {
  logger.debug("swp_protocol::sendFrame");
  swp_state.sendFrame(frame);
}

bool swp_protocol::timeoutTriggered() {
  //logger.debug("swp_protocol::timeoutTriggered");
  std::list<swp::sendQ_slot<link::Frame>>::iterator it;

  for(it = swp_state.sendQ.begin(); it != swp_state.sendQ.end(); ++it) {
    // TODO: implement a better way to compare against zero
    if (it->timeout <= 0.000001) {
      logger.debug("timeoutTriggered: True");
      return true;
    }
  }
  logger.debug("timeoutTriggered: False");
  return false;
}

void swp_protocol::timeout() {
  logger.debug("swp_protocol::timeout");
  std::list<swp::sendQ_slot<link::Frame>>::iterator it;

  for(it = swp_state.sendQ.begin(); it != swp_state.sendQ.end(); ++it) {
    // TODO: implement a better way to compare against zero
    if (it->timeout <= 0.000001) {
      it->timeout = SWP_SEND_TIMEOUT;
      to_send_frames.push(it->msg);
    }
  }
}

void swp_protocol::processFrame(link::Frame frame) {
  logger.info("swp_protocol::processFrame");
  swp::Hdr hdr;
  swp::recvQ_slot<link::Frame> slot;

  this->loadHdr(hdr, frame.preamble);
  if (hdr.Flags & FLAG_ACK_VALID) {  // received an acknowledgment---do SENDER side
    
    if (!this->inWindow(swp_state.LAR+1, SWS, hdr.AckNum)) {
      logger.info("Discarted frame: Frame not in SWS windows");
      logger.debug( "fram SeqNum: " + std::to_string(hdr.SeqNum) + 
                    " LAR+1: " + std::to_string(swp_state.LAR+1) + 
                    " SWS: " + std::to_string(SWS));
    }
    
    logger.debug( "waiting ACK frames: " + std::to_string(swp_state.sendQ.size()));
    while (swp_state.LAR != hdr.AckNum) {
      logger.debug( "ACK received for frame with SeqNum: " + 
                    std::to_string(swp_state.sendQ.front().SeqNum));
      swp_state.sendQ.pop_front();
      ++swp_state.LAR;
    };
    logger.debug("SWP Send queue size is: " + std::to_string(swp_state.sendQ.size()));
    logger.debug( "waiting ACK frames: " + std::to_string(swp_state.sendQ.size()));
    return;
  } 

  if (hdr.Flags & FLAG_HAS_DATA) { // received data packet---do RECEIVER side
    
    if (!this->inWindow(swp_state.LFR+1, RWS, hdr.SeqNum)) { // drop the message
      logger.info("Discarted frame: Frame not in RWS windows");
      logger.debug( "frame SeqNum: " + std::to_string(hdr.SeqNum) + 
                    " LFR+1: " + std::to_string(swp_state.LFR+1) + 
                    " RWS: " + std::to_string(RWS));
      return;
    }

    // the sort custom comparator is the one that uses the RWS to know where the seqno
    // has started again and if that case, put smaller seqnos at the end of the list.
    slot.msg = frame;
    slot.SeqNum = hdr.SeqNum;
    swp_state.recvQ.push_back(slot);
    swp_state.recvQ.sort(swp_protocol::recvQ_slotComparator); 
    logger.debug("Out of order frames: " + std::to_string(swp_state.recvQ.size()));
    
    while (!swp_state.recvQ.empty() && (swp_state.LFR+1) == swp_state.recvQ.front().SeqNum) {
      logger.info("Deliver frame with SeqNum: " + std::to_string(swp_state.recvQ.front().SeqNum));
      accepted_frames.push(frame);
      ++swp_state.LFR;
      swp_state.recvQ.pop_front();
    }
    
    logger.debug("Out of order frames: " + std::to_string(swp_state.recvQ.size()));
    this->sendAck(frame.MAC_source, swp_state.LFR);
  }
}

bool swp_protocol::SeqnoRWSComparator(swp::Seqno a, swp::Seqno b) {
  if (abs(a-b) <= RWS) return a < b;
  return b < a;
}

bool swp_protocol::recvQ_slotComparator(swp::recvQ_slot<link::Frame> a, swp::recvQ_slot<link::Frame> b) {
  
  return swp_protocol::SeqnoRWSComparator(a.SeqNum, b.SeqNum);
}

double swp_protocol::nexTimeout() {
  double next = infinity;
  std::list<swp::sendQ_slot<link::Frame>>::iterator it;

  for(it = swp_state.sendQ.begin(); it !=swp_state.sendQ.end(); ++it) {
    if (it->timeout < next) {
      next = it->timeout;
    }
  }
  return next;
}

void swp_protocol::storeHdr(const swp::Hdr& hdr, unsigned long& preamble) {
  preamble = preamble & 0x000FFFFF; // cleaning the bits where swp::Hdr will be placed
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

void swp_protocol::sendAck(MAC mac_dest, swp::Seqno LFR) {
  logger.info("send ACK LFR: " + std::to_string(LFR));
  link::Frame frame;
  swp::Hdr hdr;

  hdr.AckNum = LFR;
  hdr.Flags = FLAG_ACK_VALID;
  this->storeHdr(hdr,frame.preamble);

  frame.MAC_destination = mac_dest;
  frame.MAC_source = mac;
  frame.EtherType = 0; // Not currently used
  frame.disableARPFlag();

  to_send_frames.push(frame);
}

bool swp_protocol::inWindow(swp::Seqno startNum, ushort WS, swp::Seqno seqNum) {
  for (swp::Seqno i=0; i<WS; ++i) {
    swp::Seqno current = startNum+i;
    if (seqNum == current)
      return true;
  }
  return false;
}

void swp_protocol::loadHdr(swp::Hdr& hdr, const unsigned long& preamble) {
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

void swp_protocol::updateTimeouts(double elapsed_time) {
  std::list<swp::sendQ_slot<link::Frame>>::iterator it;

  for(it = swp_state.sendQ.begin(); it !=swp_state.sendQ.end(); ++it) {
    if (it->timeout != infinity)
      it->timeout -= elapsed_time;
  }
}