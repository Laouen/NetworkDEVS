#include "udp_protocol.h"

void udp_protocol::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // reading initial ips
  int ip_amount = (int)va_arg(parameters,double);
  for(int i=0;i<ip_amount;++i){
    ips.push_back(va_arg(parameters,char*));
  }

  next_internal = infinity;
  output = Event(0,5);
}

double udp_protocol::ta(double t) {
  //printLog("[in str] ta\n");
  return next_internal;
}

void udp_protocol::dint(double t) {
  //printLog("[in str] dint\n");

  if (!ntw_ctrl_in.empty()) {
    ip::Control c = ntw_ctrl_in.front();
    this->processNtwCtrl(c);
    ntw_ctrl_in.pop();
    next_internal = add_rm_ip_time;
    output = Event(0,5);
    return;
  }

  if (!app_ctrl_in.empty()) {
    app::Control c = app_ctrl_in.front();
    this->processAppCtrl(c,t);
    app_ctrl_in.pop();
    next_internal = app_ctrl_time;
    return;
  }

  if (!datagram_in.empty()) {
    udp::Datagram d = datagram_in.front();
    this->processDatagram(d,t);
    datagram_in.pop();
    next_internal = delivering_time;
    return;
  }

  output = Event(0,5);
  next_internal = infinity;
}

void udp_protocol::dext(Event x, double t) {
  switch(x.port) {
  case 1: // Application layer control message arrives
    app_ctrl_in.push(*(app::Control*)x.value);
    break;
  case 2: // Datagram from network layer arrives
    datagram_in.push(*(udp::Datagram*)x.value);
    break;
  case 3: // Network layer control message arrives
    ntw_ctrl_in.push(*(ip::Control*)x.value);
    break;
  default:
    printLog("[ERROR][udp protocol] invalid port ");
    printLog(std::to_string(x.port).c_str());
    printLog("\n");
    break;
  }

  if (next_internal < infinity)
    next_internal -= t;
  else if (this->queuedMsgs()) 
    next_internal = 0;
}

Event udp_protocol::lambda(double t) {
  return output;
}

void udp_protocol::exit() {}

//////////////////////////////////////////////////
//////////////// Helper methods //////////////////
//////////////////////////////////////////////////

/*********** Main helpers ************************/

void udp_protocol::processDatagram(const udp::Datagram& d, double t) {
  app::Packet p = d.payload;

  // Datagrams with incorrect dest_ip are thrown away
  ushort local_port = d.header.dest_port;
  ushort remote_port = d.header.src_port;
  IPv4 local_ip = d.psd_header.dest_ip;
  IPv4 remote_ip = d.psd_header.src_ip;
  if (existentSocket(local_port,local_ip) && evaluateChecksum(p,d.header.checksum)) {
    udp::Socket& s = sockets[local_port][local_ip];
    // dest is actually src
    if (s.accept(local_port,local_ip,remote_port,remote_ip)) {
      // deliver data
      udp::Control m(s.app_id, p);
      output = Event(multiplexed_out.push(m,t), 1);
      s.stopReading();
    }
  }
}

void udp_protocol::processAppCtrl(const app::Control &c, double t) {

  ushort port = c.port;
  IPv4 ip = c.ip;
  int app_id = c.app_id;
  
  udp::Socket* s;
  udp::Control m;

  switch(c.request) {


  /********* CONNECT **************/
  case app::Ctrl::CONNECT:
    printLog("[udp][API] Connect\n");
    if (this->existentIP(ip) && !this->existentSocket(port,ip)) {
      // connect socket
      sockets[port][ip] = udp::Socket(port,ip,c.remote_port,c.remote_ip,app_id);
      sockets[port][ip].state = udp::Socket::Status::CONNECTED;
      // send success
      udp::Control m(app_id,udp::Ctrl::SUCCESS);
      output = Event(multiplexed_out.push(m,t), 1);
    } else {
      // send invalid socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
    }
    break;
  

  /********* BIND **************/
  case app::Ctrl::BIND:
    printLog("[udp][API] Bind\n");
    if (this->existentIP(ip) && !this->existentSocket(port,ip)) {
      // connect socket
      sockets[port][ip] = udp::Socket(port,ip,app_id);
      sockets[port][ip].state = udp::Socket::Status::BOUND;
      // send success
      udp::Control m(app_id,udp::Ctrl::SUCCESS);
      output = Event(multiplexed_out.push(m,t), 1);
    } else {
      // send invalid socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
    }
    break;


  /********** READ_FROM/RECV_FROM ************/
  case app::Ctrl::READ_FROM:
  case app::Ctrl::RECV_FROM:
    printLog("[udp][API] Read_from/Recv_from\n");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    s->startReadingFrom(c.remote_port, c.remote_ip);
    output = Event(0,5);
    break;


  /*********** READ/RECV *************/
  case app::Ctrl::READ:    
  case app::Ctrl::RECV:    
    printLog("[udp][API] Read/Recv\n");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::CONNECTED && 
        s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    // set flag waiting data
    s->startReading();
    output = Event(0,5);
    break;


  /********** WRITE_TO/SEND_TO ************/
  case app::Ctrl::WRITE_TO:
  case app::Ctrl::SEND_TO:
    printLog("[udp][API] Write_to/Send_to\n");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    this->sendDataTo(c.data,*s,c.remote_port,c.remote_ip,t);
    break;


  /********* WRITE/SEND **************/
  case app::Ctrl::WRITE:
  case app::Ctrl::SEND:
    printLog("[udp][API] Write/Send\n");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invalid socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    s = &sockets[port][ip];
    if (s->state != udp::Socket::Status::CONNECTED && 
        s->state != udp::Socket::Status::BOUND) {
      // send invalid socket state
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET_STATE);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    this->sendData(c.data,*s,t);
    break;  


  /************** CLOSE ******************/
  case app::Ctrl::CLOSE:
    printLog("[udp][API] Close\n");
    if (!this->validAppSocket(port,ip,app_id)) {
      // send invald socket
      m = udp::Control(app_id,udp::Ctrl::INVALID_SOCKET);
      output = Event(multiplexed_out.push(m,t), 1);
      break;
    }

    sockets[port].erase(ip);
    if (sockets[port].empty()) {
      sockets.erase(port);
    }
    // send success
    m = udp::Control(app_id,udp::Ctrl::SUCCESS);
    output = Event(multiplexed_out.push(m,t), 1);
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
  }
}

/*********** Secondary helpers ************************/

void udp_protocol::sendData(const app::Packet& payload, const udp::Socket& s, double t) {

  udp::Datagram dat;
  // pseudo header fields
  dat.psd_header.src_ip = s.local_ip;
  dat.psd_header.dest_ip = s.remote_ip;
  dat.psd_header.udp_lenght = sizeof(udp::Header)+payload.length();
  // header fields
  dat.header.src_port = s.local_port;
  dat.header.dest_port = s.remote_port;
  dat.header.length = payload.length();
  dat.header.checksum = calculateChecksum(payload.c_str(),payload.length());
  // data
  dat.payload = payload;
  
  output = Event(datagrams_out.push(dat,t),2);
}

void udp_protocol::sendDataTo(const app::Packet& payload, const udp::Socket& s, ushort remote_port, IPv4 remote_ip, double t) {

  udp::Datagram dat;
  
  // pseudo header fields
  dat.psd_header.src_ip = s.local_ip;
  dat.psd_header.dest_ip = remote_ip;
  dat.psd_header.udp_lenght = sizeof(udp::Header)+payload.length();  
  // header fields
  dat.header.src_port = s.local_port;
  dat.header.dest_port = remote_port;
  dat.header.length = payload.length();
  dat.header.checksum = calculateChecksum(payload.c_str(),payload.length());
  // data
  dat.payload = payload;
  
  output = Event(datagrams_out.push(dat,t),2);
}

bool udp_protocol::evaluateChecksum(const app::Packet& payload, ushort checksum) {

  ushort new_checksum = ~calculateChecksum(payload.c_str(),payload.length());
  return new_checksum+checksum == 0xffff;
}

ushort udp_protocol::calculateChecksum(const char* payload, ushort count) {

  long sum = 0;
  char* addr = (char *)payload;

  while( count > 1 )  {
    /*  This is the inner loop */
    sum += *(ushort*)addr++;
    count -= 2;
  }

  /*  Add left-over byte, if any */
  if( count > 0 )
  sum += *(unsigned char*)addr++;

  /*  Fold 32-bit sum to 16 bits */
  while (sum>>16)
    sum = (sum & 0xffff) + (sum >> 16);

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

bool udp_protocol::queuedMsgs() {
  return  !app_ctrl_in.empty() ||
          !datagram_in.empty() || 
          !ntw_ctrl_in.empty();
}