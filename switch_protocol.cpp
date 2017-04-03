#include "switch_protocol.h"

void switch_protocol::init(double t, ...) {
  
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters,char*);
  logger.setModuleName("Switch " + module_name);

  interface_amount = (ushort)va_arg(parameters,double);
  logger.info("Interface amounts: " + std::to_string(interface_amount));

  // building forwarding table
  const char* file_path = va_arg(parameters,char*);
  Parser<sw::Forwarding_entry> parser(file_path);
  sw::Forwarding_entry parsed_line;
  if (parser.file_open()) {
    while(true) {
      try {
        parsed_line = parser.next_input();
        forwarding_table.insert({parsed_line.mac, parsed_line.interface});
      } catch(std::exception& e) {
        break; // end of file throws an exception
      }
    }
  } else {
    logger.error("Error parsing routing table file.");
  }

  logger.info("Forwarding table:");
  for (std::map<MAC,ushort>::iterator i = forwarding_table.begin(); i != forwarding_table.end(); ++i) {
    logger.info(i->first.as_string() + " | " + std::to_string(i->second));
  }
}

double switch_protocol::ta(double t) {
  return next_internal;
}

Event switch_protocol::lambda(double t) {
  return output;
}

void switch_protocol::dinternal(double t) {
  if (!lower_layer_data_in.empty()) {
    link::Multiplexed_frame frame = lower_layer_data_in.front();
    this->processFrame(frame);
    lower_layer_data_in.pop();
    next_internal = process_frame_time;
  }
}

void switch_protocol::processFrame(link::Multiplexed_frame& multiplexed_frame) {
  link::Frame frame = multiplexed_frame.frame;
  
  if (frame.MAC_destination == BROADCAST_MAC_ADDRESS) {
    this->sendToAllInterfaces(frame, multiplexed_frame.interface);
    return;
  }

  if (forwarding_table.find(frame.MAC_destination) == forwarding_table.end()) {
    // this model does not implement any dinamic method to get the interface
    logger.info("Destination mac address " + 
                frame.MAC_destination.as_string() + 
                " not found in forwarding table");
    return;
  }

  this->send(frame,forwarding_table.at(frame.MAC_destination));
}

void switch_protocol::sendToAllInterfaces(link::Frame& frame, ushort source_interface) {
  logger.info("Bradcast MAC sent throw all interfaces");
  logger.debug("srouce interface: " + std::to_string(source_interface));

  std::set<ushort> interfaces;
  std::map<MAC,ushort>::iterator it;
  for(it = forwarding_table.begin(); it != forwarding_table.end(); ++it) {
    if (it->second != source_interface) {
      interfaces.insert(it->second);
    }
  }

  std::set<ushort>::iterator jt;
  for(jt = interfaces.begin(); jt != interfaces.end(); ++jt){
    this->send(frame,*jt);
  }
}

void switch_protocol::send(link::Frame& frame, ushort interface) {
    logger.info("send frame for MAC: " + 
                frame.MAC_destination.as_string() + 
                " throw interface " + 
                std::to_string(interface));

    link::Multiplexed_frame m;
    m.interface = interface;
    m.frame = frame; 
    lower_layer_data_out.push(m,2);   
}