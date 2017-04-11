#include "dns_client_protocol.h"

void dns_client_protocol::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters, char*);
  logger.setModuleName("DNS-client " + module_name);

  client_ip = va_arg(parameters,char*);
  logger.info("DNS client ip: " + client_ip.as_string());

  local_root_server_ip = va_arg(parameters,char*);
  logger.info("DNS local root server ip: " + local_root_server_ip.as_string());

  // building authoritative RR table
  const char* file_path = va_arg(parameters,char*);
  Parser<dns::ResourceRecord> parser(file_path);
  if (parser.file_open()) {
    while(true) {
      try {
        authoritative_RRs.push_back(parser.next_input());
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    logger.error("Error parsing routing table file.");
  }

  logger.info("Authoritative RR table:");
  for (std::list<dns::ResourceRecord>::iterator i = authoritative_RRs.begin(); i != authoritative_RRs.end(); ++i) {
    logger.info(i->as_string());
  }

  next_id = 0;
  
  // starting udp bind
  bind = true;
  udp::Control c(0,udp::Ctrl::BIND);
  c.local_port = 53;
  c.local_ip = client_ip;
  lower_layer_ctrl_out.push(c,3);
  next_internal = 0;
}

void dns_client_protocol::dinternal(double) {
  if (bind) {
    bind = false;
    return;
  }

  if (!higher_layer_data_in.empty()) {
    dns::DomainName s = higher_layer_data_in.front();
    this->processDNSQuery(s);
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

double dns_client_protocol::ta(double t) { 
  return next_internal;
}

Event dns_client_protocol::lambda(double) { 
  return output;
}

void dns_client_protocol::exit() {}

/**********************************************/
/************** Helper methods ****************/
/**********************************************/

void dns_client_protocol::processDNSQuery(dns::DomainName domain) {
  
  dns::Packet p = this->wrapQueryInPacket(domain);
  if (this->existRR(domain)) {

    this->setAuthoritativeFlag(p, r.name);
    p.addAnswerResource(this->getRR(domain));
    higher_layer_data_out.push(p,0); 

  } else {

    app_requests.push_back(p);
    this->sendTo(p, local_root_server_ip, 53);

  }
}

dns::Packet dns_client_protocol::wrapQueryInPacket(dns::DomainName d) const {

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

void dns_client_protocol::sendTo(const dns::Packet& p, IPv4 server_ip, ushort server_port) {
  udp::Control c(0, udp::Ctrl::SEND_TO);
  c.remote_ip = server_ip;
  c.remote_port = server_port;

  lower_layer_data_out.push(c,2);
}

void dns_client_protocol::processDNSPacket(dns::Packet packet) {
  dns::ResourceRecord r;

  if (packet.header.is(dns::QR::QR_ANSWER,dns::QR::QR_MASK)) { // ANSWER PACKET
    
    if (this->serverError(packet)) { // RETURN ERROR
      
      this->deliverAnswer(packet);
      this->removePacket(packet.header.id);
      return;
    }

    // If it's not a server error it must have an answer
    r = packet.answers.front();
    if (r.QType == dns::Type::A) { // RETURN FOUND IPv4 DNS PACKET
      
      this->cached_RR.push_back(r);
      this->deliverAnswer(packet);  
      this->removePacket(packet.header.id);
    
    } else if (r.QType == dns::Type::NS) { // SEND REQUEST PACKET TO AUTHORITATIVE POINTED NAME SERVER

      dns::ResourceRecord ar = packet.authoritatives.front(); // authoritatives resources are allways A type 
      dns::Packet p = this->getPacket(packet.header.ip);
      this->sendTo(p, ar.AValue, 53);

    }

  } else { // QUERY PACKET
    
    r = packet.aditionals.front(); // first aditional is requester host ip
    if (this->existRR(r.name)) {

      this->setAuthoritativeFlag(p, r.name);
      p.addAnswerResource(this->getRR(domain));
      this->sendTo(p, r.AValue, 53); 

    } else {

      host_requests.push_back(p);
      this->sendTo(p, local_root_server_ip, 53);

    }

  }
}

void dns_client_protocol::setAuthoritativeFlag(dns::Packet& packet, dns::DomainName d) {}

bool dns_client_protocol::serverError(const dns::Packet& packet) const {}

bool dns_client_protocol::isAppRequest(const dns::Packet& packet) const {}

bool dns_client_protocol::isHostRequest(const dns::Packet& packet) const {}

void dns_client_protocol::removePacket(const dns::Packet& packet) {}

dns::packet dns_client_protocol::getPacket(int id) {}

bool dns_client_protocol::existRR(const dns::DomainName& d) {}

dns::ResourceRecord dns_client_protocol::getRR(const dns::DomainName& d) {}

void dns_client_protocol::deliverAnswer(const dns::Packet& packet) {

  if (this->isAppRequest(packet.header.id)) {
    higher_layer_data_out.push(packet,0);
  } else if (this->isHostRequest(packet.header.id)) {
    IPv4 host_ip = packet.aditionals.front().AValue; // first aditional is requester host ip
    this->sendResponse(packet, host_ip, 53);
  }
}