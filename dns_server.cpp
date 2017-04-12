#include "dns_server.h"

void dns_server::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters, char*);
  logger.setModuleName("DNS " + module_name);

  client_ip = va_arg(parameters,char*);
  logger.info("DNS local ip: " + client_ip.as_string());

  root_server_ip = va_arg(parameters,char*);
  logger.info("DNS root server ip: " + root_server_ip.as_string());

  // Building authoritative RR table
  const char* file_path = va_arg(parameters,char*);
  Parser<dns::ResourceRecord> parser_rr(file_path);
  if (parser_rr.file_open()) {
    logger.info("Authoritative RR table:");
    while(true) {
      try {
        authoritative_RRs.push_back(parser_rr.next_input());
        logger.info(authoritative_RRs.back().as_string());
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    logger.error("Error parsing authoritative RR table file.");
  }

  // Building authoritative RR table
  file_path = va_arg(parameters,char*);
  Parser<dns::Zone> parser_zone(file_path);
  if (parser_zone.file_open()) {
    logger.info("Zone servers table:");
    while(true) {
      try {
        zone_servers.push_back(parser_zone.next_input());
        logger.info(zone_servers.back().as_string());
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    logger.error("Error parsing zone server table file.");
  }

  // Getting recursive allowed parameter
  recursive_allowed = va_arg(parameters,double);
  if (recursive_allowed)
  	logger.info("Recursive allowed: True");
  else
  	logger.info("Recursive allowed: False");

  next_id = 0;
  
  // Starting udp bind
  bind = true;
  udp::Control c(0,udp::Ctrl::BIND);
  c.local_port = 53;
  c.local_ip = client_ip;
  lower_layer_ctrl_out.push(c,3);
  next_internal = 0;
}

void dns_server::dinternal(double t) {
  
  this->updateCache(t);

  bind = false;

  if (!higher_layer_data_in.empty()) {
    dns::DomainName s = higher_layer_data_in.front();
    this->processDomainName(s);
    higher_layer_data_in.pop();
    next_internal = process_dns_query;
    return;
  }

  if (!lower_layer_data_in.empty()) {
    dns::Packet p = lower_layer_data_in.front();
    this->processDNSPacket(p);
    lower_layer_data_in.pop();
    next_internal = process_dns_response;
    return;
  }
}

void dns_server::dexternal(double t) {
  
  this->updateCache(t);
}

double dns_server::ta(double t) { 
  
  return next_internal;
}

Event dns_server::lambda(double) { 
  return output;
}

void dns_server::exit() {}

/**********************************************/
/************** Helper methods ****************/
/**********************************************/

void dns_server::processDomainName(dns::DomainName domain) {
  
  dns::Packet p = this->QueryPacket(domain);
  if (this->existRR(domain)) {

    this->directAnswer(p);
    higher_layer_data_out.push(p,0); 

  } else {

    app_requests.push_back(p);
    this->sendTo(p, root_server_ip, 53);

  }
}

dns::Packet dns_server::QueryPacket(dns::DomainName d) const {

  // First aditional RR is the requested domain with the requester host IPv4
  dns::ResourceRecord RRRequesterIP;
  RRRequesterIP.name = d; // same name as the query
  RRRequesterIP.QType = dns::Type::A;
  RRRequesterIP.QClass = dns::Class::IN
  RRRequesterIP.AValue = client_ip; // client ip to let the server respond to this IPv4

  // query RR
  dns::ResourceRecord RRQuery;
  RRQuery.name = d;
  RRQuery.QType = dns::Type::A;
  RRQuery.QClass = dns::Class::IN;
  
  dns::Packet packet;
  packet.header.id = next_id;
  packet.header.setFlag(0x0000,0xFFFF); // set all in zero
  packet.header.setFlag(dns::QR::QR_QUERY,dsn::QR::QR_MASK);
  packet.header.setFlag(dns::Opcode::Opcode_QUERY,dsn::Opcode::Opcode_MASK);
  packet.header.setFlag(dns::AA::AA_NOT_AUTHORITATIVE_ANSWER,dsn::AA::AA_MASK);
  packet.header.setFlag(dns::TC::TC_NOT_TRUNCATED,dsn::TC::TC_MASK);
  packet.header.setFlag(dns::RD::RD_RECURSIVE,dsn::RD::RD_MASK);
  packet.addQuestionResource(RRQuery);
  packet.addAditionalResource(RRRequesterIP);

  // update dns state
  next_id++;

  return packet;
}

void dns_server::sendTo(const dns::Packet& p, IPv4 server_ip, ushort server_port) {
  udp::Control c(0, udp::Ctrl::SEND_TO);
  c.remote_ip = server_ip;
  c.remote_port = server_port;

  lower_layer_ctrl_out.push(c,2);
}

void dns_server::processDNSPacket(dns::Packet packet) {
  dns::ResourceRecord r;

  if (packet.header.is(dns::QR::QR_ANSWER,dns::QR::QR_MASK)) { // ANSWER PACKET
    
    if (!packet.header.is(dns::RCode::RCode_NO_ERROR,dns::RCode::RCode_MASK)) { // RETURN ERROR
      
      this->deliverAnswer(packet);
      this->removePacket(packet.header.id);
      return;
    }

    // If it's not a server error it must have an answer
    r = packet.answers.front();
    if (r.QType == dns::Type::A) { // RETURN FOUND IPv4 DNS PACKET
      
      if (packet.header.is(dns::AA::AA_AUTHORITATIVE_ANSWER,dns::AA::AA_MASK)) {
        this->cached_RRs.push_back(r);
      }
      this->deliverAnswer(packet);  
      this->removePacket(packet.header.id);
    
    } else if (r.QType == dns::Type::NS) { // SEND REQUEST PACKET TO AUTHORITATIVE POINTED NAME SERVER

      dns::ResourceRecord ar = packet.authoritatives.front(); // authoritatives resources are allways A type 
      dns::Packet p = this->getPacket(packet.header.ip);
      this->sendTo(p, ar.AValue, 53);

    }

  } else { // QUERY PACKET
    
    r = packet.aditionals.front(); // first aditional is requester host ip

    if (this->existRR(r.name)) { // RESOURCE EXIST

      this->directAnswer(packet);
      this->sendTo(packet, r.AValue, 53);

    } else if (packet.header.is(dns::RD::RD_RECURSIVE,dsn::RD::RD_MASK) && recursive_allowed) { // RECURSIVE QUERY

      host_requests.push_back(packet);
      this->sendTo(packet, root_server_ip, 53);

    } else { // RETURN BEST MATCH OR NAME ERROR

      dns::Zone zone = this->getBestMatch(r.name);

      if (!zone.empty()) {

        packet.answers.addAnswerResource(zone.name_server);
        packet.answers.addAuthoritativeResource(zone.authoritative);
        packet.header.setFlag(dns::RCode::RCode_NO_ERROR,dns::RCode::RCode_MASK);
      
      } else {

        packet.header.setFlag(dns::RCode::RCode_NAME_ERROR,dns::RCode::RCode_MASK);
      }

      this->setAsResponse(packet);
      packet.header.setFlag(dns::AA::AA_NOT_AUTHORITATIVE_ANSWER,dns::AA::AA_MASK);
      this->sendTo(packet, r.AValue, 53);
    }
  }
}

void dns_server::setAsResponse(dns::Packet& packet) {
  packet.header.setFlag(dns::QR::QR_ANSWER,dns::QR::QR_MASK);
  packet.header.setFlag(dns::TC::TC_NOT_TRUNCATED,dns::TC::TC_MASK);
}

void dns_server::directAnswer(dns::Packet& packet) {

  this->setAsResponse(packet);
  packet.header.setFlag(dns::RCode::RCode_NO_ERROR,dns::RCode::RCode_MASK);
  
  dns::DomainName d = packet.aditionals.front().name;
  this->setAuthoritativeFlag(packet, d);
  packet.addAnswerResource(this->getRR(d));
}

void dns_server::deliverAnswer(const dns::Packet& packet) {

  if (this->isAppRequest(packet.header.id)) {
    higher_layer_data_out.push(packet,0);
  } else if (this->isHostRequest(packet.header.id)) {
    IPv4 host_ip = packet.aditionals.front().AValue; // first aditional is requester host ip
    this->sendResponse(packet, host_ip, 53);
  }
}

void dns_server::setAuthoritativeFlag(dns::Packet& packet, dns::DomainName d) {
  std::list<dns::ResourceRecord>::const_iterator it;
  
  for(it = cached_RRs.begin(); it != cached_RRs.end(); ++it) {
    if (it->name == d.name) {
      packet.header.setFlag(dns::AA::AA_NOT_AUTHORITATIVE_ANSWER,dns::AA::AA_MASK);
      return;
    }
  }

  for(it = authoritative_RRs.begin(); it != authoritative_RRs.end(); ++it) {
    if (it->name == d.name) {
      packet.header.setFlag(dns::AA::AA_AUTHORITATIVE_ANSWER,dns::AA::AA_MASK);
      return;
    }
  }
}

bool dns_server::isAppRequest(ushort id) const {
  std::list<dns::Packet>::const_iterator it;
  for(it = app_requests.begin(); it != app_requests.end(); ++it) {
    if (it->header.id == id) return true;
  }
  return false;
}

bool dns_server::isHostRequest(ushort id) const {
  std::list<dns::Packet>::const_iterator it;
  for(it = host_requests.begin(); it != host_requests.end(); ++it) {
    if (it->header.id == id) return true;
  }
  return false;
}

void dns_server::removePacket(const dns::Packet& packet) {
  std::list<dns::Packet>::const_iterator it;

  // Removing from app request
  it = app_requests.begin();
  while(it != app_requests.end()) {
    if (it->header.id == id) {
      it = app_requests.erase(it);
    } else {
      ++it;
    }
  }

  // Removing from host request
  it = host_requests.begin();
  while(it != host_requests.end()) {
    if (it->header.id == id) {
      it = host_requests.erase(it);
    } else {
      ++it;
    }
  }
}

dns::packet dns_server::getPacket(ushort id) {
  std::list<dns::Packet>::const_iterator it;

  for(it = app_requests.begin(); it != app_requests.end(); ++it) {
    if (it->header.id == id) return *it;
  }

  for(it = host_requests.begin(); it != host_requests.end(); ++it) {
    if (it->header.id == id) return *it;
  }
}

bool dns_server::existRR(const dns::DomainName& d) const {

  std::list<dns::ResourceRecord>::const_iterator it;
  for(it = cached_RRs.begin(); it != cached_RRs.end(); ++it) {
    if (it->name == d.name) return true;
  }

  for(it = authoritative_RRs.begin(); it != authoritative_RRs.end(); ++it) {
    if (it->name == d.name) return true;
  }

  return false;
}

dns::ResourceRecord dns_server::getRR(const dns::DomainName& d) {
  std::list<dns::ResourceRecord>::const_iterator it;

  for(it = cached_RRs.begin(); it != cached_RRs.end(); ++it) {
    if (it->name == d.name) return *it;
  }

  for(it = authoritative_RRs.begin(); it != authoritative_RRs.end(); ++it) {
    if (it->name == d.name) return *it;
  }
}

void dns_server::updateCache(double t) {
  ushort elapsed_time = std::floor(t-last_transition);

  std::list<dns::ResourceRecord>::const_iterator it = cached_RRs.begin();
  while(it != cached_RRs.end()) {
    it->TTL -= elapsed_time; 
    if (it->TTL <= 0) {
      it = cached_RRs.erase(it);
    } else {
      ++it;
    }
  }
}

dns::Zone dns_server::getBestMatch(dns::DomainName d) {
  dns::Zone z;
  std::list<dns::ResourceRecord>::const_iterator it;

  for(it = zone_servers.begin(); it != zone_servers.end(); ++it) {
    if (it->isZoneOf(d) && it->isZoneOf(z.authoritative.name)) {
      z = *it;
    }
  }

  return z;
}