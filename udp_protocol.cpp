#include "udp_protocol.h"

void udp_protocol::init(double t,...) {

  //TODO: Check for duplicated packets that arrives and must be discarted
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters,char*);
  logger.setModuleName("UDP " + module_name);

  // reading initial ips
  int ip_amount = (int)va_arg(parameters,double);
  for(int i=0;i<ip_amount;++i){
    ips.push_back(va_arg(parameters,char*));
  }
  
  next_internal = infinity;
  output = Event(0,5);
}

double udp_protocol::ta(double t) {
  return next_internal;
}

void udp_protocol::dinternal(double t) {

  if (!lower_layer_ctrl_in.empty()) {
    ip::Control c = lower_layer_ctrl_in.front();
    this->processNtwCtrl(c);
    lower_layer_ctrl_in.pop();
    next_internal = add_rm_ip_time;
    return;
  }

  if (!higher_layer_ctrl_in.empty()) {
    udp::Control c = higher_layer_ctrl_in.front();
    this->processUDPCtrl(c,t);
    higher_layer_ctrl_in.pop();
    next_internal = app_ctrl_time;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    udp::Segment d = lower_layer_data_in.front();
    this->processSegment(d,t);
    lower_layer_data_in.pop();
    next_internal = delivering_time;
    return;
  }
}

Event udp_protocol::lambda(double t) {
  return output;
}

void udp_protocol::exit() {}

//////////////////////////////////////////////////
//////////////// Helper methods //////////////////
//////////////////////////////////////////////////

/*********** Main helpers ************************/

void udp_protocol::processSegment(const udp::Segment& d, double t) {
  dns::Packet p(d.payload);

  // Segments with incorrect dest_ip are thrown away
  ushort local_port = d.header.dest_port;
  ushort remote_port = d.header.src_port;
  IPv4 local_ip = d.psd_header.dest_ip;
  IPv4 remote_ip = d.psd_header.src_ip;
  logger.debug("process Segment: ");
  logger.debug("local_port: " + std::to_string(local_port));
  logger.debug("remote_port: " + std::to_string(remote_port));
  logger.debug("local_ip: " + local_ip.as_string());
  logger.debug("remote_ip: " + remote_ip.as_string());
  if (existentSocket(local_port,local_ip) && verifyChecksum(d)) {
    udp::Socket& s = sockets[local_port][local_ip];
    logger.debug("socket local_ip: " + s.local_ip.as_string());
    logger.debug("socket local_ip: " + std::to_string(s.local_port));
    // dest is actually src
    if (s.accept(local_port,local_ip,remote_port,remote_ip)) {
      // deliver data
      logger.info("Deliver dns packet");
      higher_layer_data_out.push(p, 0);
      s.stopReading();
    } else {
      logger.info("Not accepted packet");
    }
  } else {
    logger.info("discarted segment.");
  }
}

void udp_protocol::processUDPCtrl(const udp::Control &c, double t) {

  ushort port = c.local_port;
  IPv4 ip = c.local_ip;
  int app_id = c.app_id;
  
  udp::Socket* s;
  udp::Control m;

  switch(c.request) {


  /********* CONNECT **************/
  case udp::Ctrl::CONNECT:
    logger.info("Connect");
    if (this->existentIP(ip) && !this->existentSocket(port,ip)) {
      // connect socket
      sockets[port][ip] = udp::Socket(port,ip,c.remote_port,c.remote_ip,app_id);
      sockets[port][ip].state = udp::Socket::Status::CONNECTED;
      // send success
      logger.debug("SUCCESS");
      m = udp::Control(app_id,udp::Ctrl::SUCCESS);
    } else {
      // send invalid socket
      logger.debug("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
    }
    higher_layer_ctrl_out.push(m, 1);
    break;
  

  /********* BIND **************/
  case udp::Ctrl::BIND:
    logger.info("Bind");
    if (this->existentIP(ip) && !this->existentSocket(port,ip)) {
      // connect socket
      sockets[port][ip] = udp::Socket(port,ip,app_id);
      sockets[port][ip].state = udp::Socket::Status::BOUND;
      // send success
      logger.debug("SUCCESS");
      m = udp::Control(app_id,udp::Ctrl::SUCCESS);
    } else {
      // send invalid socket
      logger.debug("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
    }
      higher_layer_ctrl_out.push(m, 1);
    break;


  /********** READ_FROM/RECV_FROM ************/
  case udp::Ctrl::READ_FROM:
  case udp::Ctrl::RECV_FROM:
    logger.info("Read_from/Recv_from");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      logger.debug("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    s->startReadingFrom(c.remote_port, c.remote_ip);
    break;


  /*********** READ/RECV *************/
  case udp::Ctrl::READ:    
  case udp::Ctrl::RECV:    
    logger.info("Read/Recv");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      logger.info("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::CONNECTED && 
        s->state != udp::Socket::Status::BOUND) {
      logger.info("INVALID_SOCKET_STATE");
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    // set flag waiting data
    s->startReading();
    output = Event(0,5);
    break;


  /********** WRITE_TO/SEND_TO ************/
  case udp::Ctrl::WRITE_TO:
  case udp::Ctrl::SEND_TO:
    logger.info("Write_to/Send_to");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      logger.debug("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    this->sendDataTo(c.packet,*s,c.remote_port,c.remote_ip,t);
    break;


  /********* WRITE/SEND **************/
  case udp::Ctrl::WRITE:
  case udp::Ctrl::SEND:
    logger.info("Write/Send");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      logger.debug("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::CONNECTED && 
        s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    this->sendData(c.packet,*s,t);
    break;  


  /************** CLOSE ******************/
  case udp::Ctrl::CLOSE:
    logger.info("Close");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invald socket
      logger.debug("INVALID_SOCKET");
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      higher_layer_ctrl_out.push(m, 1);
      break;
    }

    sockets[port].erase(ip);
    if (sockets[port].empty()) {
      sockets.erase(port);
    }
    // send success
    logger.debug("SUCCESS");
    m = udp::Control(app_id,udp::Ctrl::SUCCESS);
    higher_layer_ctrl_out.push(m, 1);
    break;
  default:
    logger.error("Process udp::Control bad case.");
    break;
  }
}

void udp_protocol::processNtwCtrl(const ip::Control &c) {
  switch(c.request) {
  case ip::Ctrl::ADD_IP:
    if (!this->existentIP(c.ip))
      ips.push_back(c.ip);
    break;
  case ip::Ctrl::REMOVE_IP:
    for (std::vector<IPv4>::iterator i = ips.begin(); i != ips.end(); ++i) {
      if (c.ip == *i) ips.erase(i,i+1);
    }
    break;
  case ip::Ctrl::ROUTING_ERROR:
    //TODO: send the error to the upper layer
    logger.info("Routing error received from network layer.");
    break;
  default:
    logger.error("Process ip::Control bad case.");
    break;
  }
}

/*********** Secondary helpers ************************/


// TODO: merge sendData and sendDataTo equal codes to a single function send.
void udp_protocol::sendData(const dns::Packet& payload, const udp::Socket& s, double t) {

  udp::Segment dat;
  // pseudo header fields
  dat.psd_header.src_ip = s.local_ip;
  dat.psd_header.dest_ip = s.remote_ip;
  dat.psd_header.udp_lenght = sizeof(udp::Header)+payload.size();
  dat.psd_header.zeros = 0x0;
  dat.psd_header.protocol = 0x0; // Currently not used
  // header fields
  dat.header.src_port = s.local_port;
  dat.header.dest_port = s.remote_port;
  dat.header.length = payload.size();
  dat.header.checksum = calculateChecksum(dat);
  // data
  dat.setPayload(payload);

  lower_layer_data_out.push(dat, 2);
}

void udp_protocol::sendDataTo(const dns::Packet& payload, const udp::Socket& s, ushort remote_port, IPv4 remote_ip, double t) {

  udp::Segment dat;

  // pseudo header fields
  dat.psd_header.src_ip = s.local_ip;
  dat.psd_header.dest_ip = remote_ip;
  dat.psd_header.udp_lenght = sizeof(udp::Header)+payload.size();
  dat.psd_header.zeros = 0x0;
  dat.psd_header.protocol = 0x0; // Currently not used
  // header fields
  dat.header.src_port = s.local_port;
  dat.header.dest_port = remote_port;
  dat.header.length = payload.size();
  dat.header.checksum = calculateChecksum(dat);
  // data
  dat.setPayload(payload);

  lower_layer_data_out.push(dat, 2);
}

bool udp_protocol::verifyChecksum(udp::Segment d) const {
  ushort calculated_checksum = ~calculateChecksum(d);
  
  logger.debug("Header checksum: " + std::to_string(d.header.checksum));
  logger.debug("Calculated checksum: " + std::to_string(calculated_checksum));
  
  bool valid_checksum = calculated_checksum+d.header.checksum == 0xFFFF;

  if (valid_checksum)
    logger.info("valid checksum: true");
  else
    logger.info("valid checksum: False");

  return valid_checksum;
}

ushort udp_protocol::calculateChecksum(udp::Segment d) const {

  const char* header_ptr = d.headers_c_str();
  ushort count = d.headers_size();

  long sum = 0;
  char* addr = (char *)header_ptr;
  while( count > 1 )  {
    /*  This is the inner loop */
    sum += *(ushort*)addr++;
    count -= 2;
  }

  /*  Add left-over byte, if any */
  if( count > 0 )
  sum += *(unsigned char*)addr++;

  /*  Fold 32-bit sum to 16 bits */
  while (sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  delete[] header_ptr;
  return ~sum;
}

bool udp_protocol::existentIP(IPv4 ip) {
  for (std::vector<IPv4>::iterator i = ips.begin(); i != ips.end(); ++i) {
    if (ip == *i) return true;
  }

  return false;
}

bool udp_protocol::existentSocket(ushort port, IPv4 ip) {
  std::map<ushort,std::map<IPv4,udp::Socket>>::iterator port_it = sockets.find(port);
  return  port_it != sockets.end() && port_it->second.find(ip) != port_it->second.end();
}

bool udp_protocol::validAppSocket(ushort port, IPv4 ip, int app_id) {
  return  this->existentSocket(port,ip) && sockets[port][ip].app_id == app_id;
}