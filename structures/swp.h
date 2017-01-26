#if !defined swp_h
#define swp_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#define SWS 4
#define RWS 4

namespace swp {

  u_char FLAG_ACK_VALID = 0x1; // TODO: check what number should go here;
  u_char FLAG_HAS_DATA = 0x2; // TODO: check what number should go here;
  double SWP_SEND_TIMEOUT = 1; // TODO: check what number should go here
  typedef u_char SwpSeqno;

  struct SwpHdr {
    SwpSeqno SeqNum; /* sequence number of this frame */
    SwpSeqno AckNum; /* ack of received frame */
    u_char Flags; /* up to 8 bits worth of flags */
  };

  
  template <typename MSG>
  struct SwpState {
    /* sender side state: */
    SwpSeqno LAR; /* seqno of last ACK received */
    SwpSeqno LFS; /* last frame sent */
    u_char sendWindowEmptyPlaces; /* this is normally a semaphore called sendWindowNotFull, 
    But for the purpose of a the DEVS model it's best implemented using a counter and 
    there is no need to be implemeted as semaphore and it add unessesary complexity */
    SwpHdr hdr; /* pre-initialized header */
  
    struct sendQ_slot {
      double timeout; /* event associated with send timeout, this field is usually a Event type
      but for the DEVS model, a double time is used */
      MSG msg;
    } sendQ[SWS];

    /* receiver side state: */
    SwpSeqno NFE; /* seqno of next frame expected */
    struct recvQ_slot {
      bool received; /* is msg valid? */
      MSG msg;
    } recvQ[RWS];
  };
}

inline std::ostream& operator<<(std::ostream& os, const swp::SwpHdr& hdr) {
  os << "SeqNum: " << (int)hdr.SeqNum << std::endl;
  os << "AckNum: " << (int)hdr.AckNum << std::endl;
  os << "Flags: " << (int)hdr.Flags << std::endl;
  return os;
}

#endif