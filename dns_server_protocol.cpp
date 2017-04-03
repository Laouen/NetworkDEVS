#include "dns_server_protocol.h"

void dns_server_protocol::init(double t,...) {
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters, char*);
  logger.setModuleName("DNS-Server " + module_name);

  // Set if recursive
  recursive = (bool)va_arg(parameters,double);
  if (recursive) logger.info("Recursive server: True"); 
  else logger.info("Recursive server: False"); 

  // Reading dns resources records
  const char* file_path = va_arg(parameters,char*);
  Parser<dns::ResourceRecord> parser(file_path);
  if (parser.file_open()) {
    while(true) {
      try {
        dns_table.push_back(parser.next_input());
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    logger.error("Error parsing DNS table file.");
  }

  logger.info("DNS table:");
  for (std::list<dns::ResourceRecord>::iterator i = dns_table.begin(); i != dns_table.end(); ++i) {
    logger.info(i->as_string());
  }

  server_ip = va_arg(parameters,char*);
  logger.info("DNS server ip: " + server_ip.as_string());

  // starting udp conextion to start listening
  starting = true;
  udp::Control c(0,udp::Ctrl::BIND);
  c.local_port = 53;
  c.local_ip = server_ip;
  lower_layer_ctrl_out.push(c,3);
}

void dns_server_protocol::dinternal(double) {
  if (starting) {
    starting = false;
    return;
  }

  output = Event(0,5);
  next_internal = infinity;
}

double dns_server_protocol::ta(double t) { 
  if (starting) return 0;
  else return next_internal;
}

Event dns_server_protocol::lambda(double) { 
  return output;
}

void dns_server_protocol::exit() {}