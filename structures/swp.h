#if !defined swp_h
#define swp_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <stdlib.h> // abs
#include <list>
#include <queue>

#define SWS 4
#define RWS 4

#define FLAG_ACK_VALID 0x1 // TODO: check what number should go here;
#define FLAG_HAS_DATA 0x2 // TODO: check what number should go here;
#define SWP_SEND_TIMEOUT 900 // TODO: check what number should go here, currently i put 900 seg = 15 min

namespace swp {

  typedef u_char Seqno;

  struct Hdr {

    Hdr() {
      SeqNum = 0x0;
      AckNum = 0x0;
      Flags = 0x0; 
    }
    Seqno SeqNum; // sequence number of this frame
    Seqno AckNum; // ack of received frame
    u_char Flags; // up to 8 bits worth of flags
  };

  template <typename MSG>
  struct sendQ_slot {
    double timeout; 
    MSG msg;
    Seqno SeqNum;
  };

  template <typename MSG>
  struct recvQ_slot {
    Seqno SeqNum;
    MSG msg;
  };
  
  template <typename MSG>
  struct State {
    // queued frames waiting to be send
    std::queue<link::Frame> framesToSend;

    // sender side state:
    Seqno LAR; // seqno of Last ACK Received
    Seqno LFS; // Last Frame Sent
    std::list<sendQ_slot<MSG>> sendQ;

    // receiver side state:
    Seqno LFR; // Last Frame Received
    std::list<recvQ_slot<MSG>> recvQ;

    State() {
      std::queue<link::Frame> empty;
      framesToSend.swap(empty);
      sendQ.clear();
      recvQ.clear();
      LFS = 0;
      LAR = 0;
      LFR = 0;
    }

    State(const State& other) {
      framesToSend = other.framesToSend;
      LAR = other.LAR;
      LFS = other.LFS;
      sendQ = other.sendQ;
      LFR = other.LFR;
      recvQ = other.recvQ;
    }

    bool sendWindowsIsFull() const {
      return sendQ.size() >= SWS;
    }

    bool thereIsFrameToSend() const {
      return !framesToSend.empty() && !this->sendWindowsIsFull();
    }

    void sendFrame(link::Frame frame) {
      framesToSend.push(frame);
    }

    void reset() {
      std::queue<link::Frame> empty;
      framesToSend.swap(empty);
      sendQ.clear();
      recvQ.clear();
      LFS = 0;
      LAR = 0;
      LFR = 0;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const swp::Hdr& hdr) {
  os << "SeqNum: " << (int)hdr.SeqNum << std::endl;
  os << "AckNum: " << (int)hdr.AckNum << std::endl;
  os << "Flags: " << (int)hdr.Flags << std::endl;
  return os;
}

#endif