#if !defined swp_h
#define swp_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <stdlib.h> // abs
#include <list>

#define SWS 4
#define RWS 4

#define FLAG_ACK_VALID 0x1 // TODO: check what number should go here;
#define FLAG_HAS_DATA 0x2 // TODO: check what number should go here;
#define SWP_SEND_TIMEOUT 900 // TODO: check what number should go here, currently i put 900 seg = 15 min

namespace swp {

  typedef u_char SwpSeqno;

  struct SwpHdr {

    SwpHdr() {
      SeqNum = 0x0;
      AckNum = 0x0;
      Flags = 0x0; 
    }
    SwpSeqno SeqNum; // sequence number of this frame
    SwpSeqno AckNum; // ack of received frame
    u_char Flags; // up to 8 bits worth of flags
  };

  template <typename MSG>
  struct sendQ_slot {
    double timeout; 
    MSG msg;
    SwpSeqno SeqNum;
  };

  template <typename MSG>
  struct recvQ_slot {
    SwpSeqno SeqNum;
    MSG msg;
  };
  
  template <typename MSG>
  struct SwpState {
    // sender side state:
    SwpSeqno LAR; // seqno of Last ACK Received
    SwpSeqno LFS; // Last Frame Sent
    std::list<sendQ_slot<MSG>> sendQ;

    // receiver side state:
    SwpSeqno LFR; // Last Frame Received
    std::list<recvQ_slot<MSG>> recvQ;
  };
}

inline std::ostream& operator<<(std::ostream& os, const swp::SwpHdr& hdr) {
  os << "SeqNum: " << (int)hdr.SeqNum << std::endl;
  os << "AckNum: " << (int)hdr.AckNum << std::endl;
  os << "Flags: " << (int)hdr.Flags << std::endl;
  return os;
}

#endif