#if !defined swp_h
#define swp_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <stdlib.h> // abs
#include <list>
#include <queue>

#define SWS 4
#define RWS 4

#define FLAG_ACK_VALID 0x1 
#define FLAG_HAS_DATA 0x2 
#define SWP_SEND_TIMEOUT 90000000 // 900000 miliseconds = 15 min

/**
 * @namespace swp
 * 
 * This namespace is used to declare al the datatypes related to the SWP protocol
 */
namespace swp {

  typedef u_char Seqno;

  /**
   * 
   * @author Laouen Louan Mayal Belloli 
   * @date 14 May 2017
   * 
   * @struct swp::Hdr swp.h
   *  
   * @brief This structs declares a Hdr as explained in the book: Computer Networks A system approach - 2011.
   * @details A Hdr has the next fields:
   * Seqno SeqNum: sequence number of this frame
   * Seqno AckNum: ack of received frame
   * u_char Flags: up to 8 bits worth of flags
   */
  struct Hdr {

    /**
     * @brief Default constructor
     * @details Construct a reseted new instance.
     */
    Hdr() {
      SeqNum = 0x0;
      AckNum = 0x0;
      Flags = 0x0; 
    }
    Seqno SeqNum;
    Seqno AckNum;
    u_char Flags;
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct swp::sendQ_slot swp.h
   * 
   * @brief This strict declares a sendQ_slot as explained in the book: Computer Networks A system approach - 2011.
   * @details This implementation differs from the one of the book because this 
   * was changed to work with DEVS and PowerDEVS.
   * 
   * @tparam MSG the datatype of the send messages.
   */
  template <typename MSG>
  struct sendQ_slot {
    double timeout; 
    MSG msg;
    Seqno SeqNum;
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct swp::recvQ_slot swp.h
   * 
   * @brief This strict declares a sendQ_slot as explained in the book: Computer Networks A system approach - 2011.
   * @details This implementation differs from the one of the book because this 
   * was changed to work with DEVS and PowerDEVS.
   * 
   * @tparam MSG the datatype of the recived messages.
   * 
   */
  template <typename MSG>
  struct recvQ_slot {
    Seqno SeqNum;
    MSG msg;
  };
  
  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct swp::state swp.h
   * 
   * @brief This strict declares a swp::state as explained in the book: Computer Networks A system approach - 2011.
   * @details This implementation differs from the one of the book because this 
   * was changed to work with DEVS and PowerDEVS.
   * 
   * @tparam MSG The datatype of the send/recived messages.
   */
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

    /**
     * @brief Default constructor.
     * @details Construct a restarted instance.
     */
    State() {
      std::queue<link::Frame> empty;
      framesToSend.swap(empty);
      sendQ.clear();
      recvQ.clear();
      LFS = 0;
      LAR = 0;
      LFR = 0;
    }

    /**
     * @brief Copy constructor
     * @details Create a copy of the instance passed as parameter in the new instace.
     * 
     * @param other The swp::State to copy in the new instance.
     */
    State(const State& other) {
      framesToSend = other.framesToSend;
      LAR = other.LAR;
      LFS = other.LFS;
      sendQ = other.sendQ;
      LFR = other.LFR;
      recvQ = other.recvQ;
    }

    /**
     * @brief Checks whether no more MSGs can be send until a new Ack arrives 
     * @return True if no more MSGs can be send, False otherwise.
     */
    bool sendWindowsIsFull() const {
      return sendQ.size() >= SWS;
    }

    /**
     * @brief Checks whether there is MSGs waiting to be send.
     * @return True if there is MSGs to send, False otherwise.
     */
    bool thereIsFrameToSend() const {
      return !framesToSend.empty() && !this->sendWindowsIsFull();
    }

    /**
     * @brief Sends a link::Frame
     */
    void sendFrame(link::Frame frame) {
      framesToSend.push(frame);
    }

    /**
     * @brief Resets the instance values to start from the begining.
     */
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