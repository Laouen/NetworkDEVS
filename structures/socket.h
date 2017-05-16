#if !defined socket_h
#define socket_h

#include <string>
#include "ipv4.h"


/**
 * @namespace udp
 * 
 * This namespace is used to declare all the UDP protocol related datatypes
 */
namespace udp {

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct udp::Socket socket.h
   * 
   * @brief This struct handles a socket states.
   * @details If the socket is connected to a remoto host, the socket has the 
   * remote ip and port must be defined defined.
   * 
   * The UDP protocol uses this struct to manage all its socket states.
   * 
   */
  struct Socket {

    /**
     * @enum udp::Socket::Status
     * 
     * All the socket valid status.
     */
    enum Status { IDLE,CONNECTED,BOUND,LISTENING };

    ushort local_port = 0;
    ushort remote_port = 0;
    IPv4 local_ip;
    IPv4 remote_ip;

    int app_id;
    Status state = Status::IDLE;
    
    // flags
    bool reading = false;
    bool reading_from = false;

    /**
     * @brief Default constructor.
     * @details Construct an empty instance.
     */
    Socket() {
      app_id = 0;
    }

    /**
     * @brief Constructor binded to an application ID
     * @details Construct a new socket with application ID specified in the parameter.
     * 
     * @param aid An int with the app ID binded to the socket
     */
    Socket(int aid) {
      app_id = aid;
    }

    /**
     * @brief Bind constructor
     * @details Construct a socket specifying its local ip and port and 
     * the bound application ID.
     * 
     * @param o_local_port An unsigned short with the socket local port.
     * @param o_local_ip An IPv4 with the socket local ip.
     * @param o_app_id An int with the application ID.
     */
    Socket(ushort o_local_port, IPv4 o_local_ip, int o_app_id) {
      app_id = o_app_id;
      local_port = o_local_port;
      local_ip = o_local_ip;
    }

    /**
     * @brief Connect constructor
     * @details Construct a new socket connect to local and remote ips an ports. 
     * It also bounds the socket to an application ID.
     * 
     * @param o_local_port An unsigned short with the socket local port.
     * @param o_local_ip IPv4 with the socket local ip.
     * @param o_remote_port An unsigned short with the socket remote port.
     * @param o_remote_ip IPv4 with the socket remote ip.
     * @param o_app_id An int woth the application ID.
     */
    Socket(ushort o_local_port, IPv4 o_local_ip, ushort o_remote_port, IPv4 o_remote_ip, int o_app_id) {
      app_id = o_app_id;
      local_port = o_local_port;
      local_ip = o_local_ip;
      remote_port = o_remote_port;
      remote_ip = o_remote_ip;
    }

    /**
     * @brief Copy constructor
     * @details Construct a new instance with a copy of the instance passed as parameter.
     * 
     * @param o An udp::Socket to copy in the new instance.
     */
    Socket(const Socket& o) {
      app_id = o.app_id;
      local_port = o.local_port;
      local_ip = o.local_ip;
      remote_port = o.remote_port;
      remote_ip = o.remote_ip;
      state = o.state;
      reading = o.reading;
      reading_from = o.reading_from;
    }

    /**
     * @brief Checks whether the socket is in status connected or not
     * @return True if the socket is connected, False otherwise.
     */
    bool connected() {
      return state == Status::CONNECTED;
    }

    /**
     * @brief Checks whether the socket is in status bound or not
     * @return True if the socket is bound, False otherwise.
     */
    bool bound() {
      return state == Status::BOUND;
    }

    /**
     * @brief Sets the socket in reading mode to accept the next incoming 
     * udp::Segment to the socket.
     * 
     * @details If the socket is in state bound, the socket accept all the 
     * udp::Segment directed to the local ip and port. If the socket is in state
     * connected, it accept only udp::Segment from remote socket as the one
     * specified in the socket.
     */
    void startReading() {
      reading = true;
    }

    /**
     * @brief Sets the socket in reading mode to accept the next incoming 
     * udp::Segment to the socket coming from the specific remote socket.
     * 
     * @details The udp::Segment that arrives for the socket must comes from a 
     * remote soket with the port and IP passed as parameters.
     */
    void startReadingFrom(ushort port, IPv4 ip) {
      remote_port = port;
      remote_ip = ip;
      reading_from = true;
    }

    /**
     * @brief Stop reading mode. No UDP::Segment is accepted.
     */
    void stopReading() {
      reading = false;
      reading_from = false;

      if (state == Status::BOUND) {
        remote_port = 0;
        remote_ip = IPv4();
      }
    }

    /**
     * @brief Checks whether the passed port and IP are the same as the socket
     * local port and IP.
     * 
     * @param port An unsigned short with the port to check.
     * @param ip An IPv4 with the IP to check.  
     * 
     * @return True if both, port and ip are the same as the socket local port and ip.
     */
    bool validLocalAddress(ushort port, IPv4 ip) {
      return  port == local_port && ip == local_ip;
    }

    /**
     * @brief Checks whether the passed port and IP are the same as the socket
     * remote port and IP.
     * 
     * @param port An unsigned short with the port to check.
     * @param ip An IPv4 with the IP to check.  
     * 
     * @return True if both, port and ip are the same as the socket remote port and ip.
     */
    bool validRemoteAddress(ushort port, IPv4 ip) {
      return port == remote_port && ip == remote_ip;
    }

    /**
     * @brief Checks whether the socket accepts udp::Segments from the remote 
     * socket with remote and local ips specified in the parameters.
     * 
     * @details The remote and local sockets parameters refers to the current 
     * socket and not to the remote one.
     * 
     * @param local_port An unsigned short with the port to be compared agains the socket local port.
     * @param local_ip An IPv4 with the ip to be compared agains the socket local ip.
     * @param remote_port An unsigned short with the port to be compared agains the socket remote port.
     * @param remote_ip An IPv4 with the ip to be compared agains the socket remote ip.
     * @return True if the remote scoket information refers to a socket for which the current socket 
     * is reading udp::Segments, False otherwise.
     */
    bool accept(ushort local_port, IPv4 local_ip, ushort remote_port, IPv4 remote_ip) {
      if (validLocalAddress(local_port,local_ip)) {
        if (bound()) {
          return  reading || 
                  (
                    reading_from && 
                    validRemoteAddress(remote_port,remote_ip)
                  );
        }

        if (connected() && reading && validRemoteAddress(remote_port,remote_ip)) {
          return true;
        }
      }

      return false;
    }
  };
}

#endif