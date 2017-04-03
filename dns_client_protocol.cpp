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

  local_server_ip = va_arg(parameters,char*);
  logger.info("DNS local name server ip: " + local_server_ip.as_string());

  // starting udp conextion to start listening
  starting = true;
  udp::Control c(0,udp::Ctrl::CONNECT);
  c.local_port = 80;
  c.remote_port = 53;
  c.local_ip = client_ip;
  c.remote_ip = local_server_ip;
  lower_layer_ctrl_out.push(c,3);
}

void dns_client_protocol::dinternal(double) {
  if (starting) {
    starting = false;
    return;
  }

  if (!higher_layer_data_in.empty()) {
    std::string s = higher_layer_data_in.front();
    this->processDNSQuery(s);
    higher_layer_data_in.pop();
    next_internal = process_dns_query;
    return;
  }
}

double dns_client_protocol::ta(double t) { 
  if (starting) return 0;
  else return next_internal;
}

Event dns_client_protocol::lambda(double) { 
  return output;
}

void dns_client_protocol::exit() {}

/**********************************************/
/************** Helper methods ****************/
/**********************************************/

void dns_client_protocol::processDNSQuery(std::string domain) {
  
  if (requests.find(domain) == requests.end()) {

    requests.push_back(domain);
    udp::Control c(0,udp::Ctrl::SEND);
    c.packet = "Poner el dns packet posta";
    lower_layer_ctrl_out.push(c,3);
  }
}

void dns_client_protocol::processDNSResponse(dns::Packet response) {}