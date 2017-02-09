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
  std::pair<double,sw::Forwarding_entry> parsed_line;
  if (parser.file_open()) {
    while(true) {
      try {
        parsed_line = parser.next_input();
        forwarding_table[parsed_line.second.mac] = parsed_line.second.interface;
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
    link::Frame frame = lower_layer_data_in.front();
    this->processFrame(frame);
    lower_layer_data_in.pop();
    next_internal = process_frame_time;
  }
}

void switch_protocol::processFrame(link::Frame& frame) {
  if (frame.MAC_destination == BROADCAST_MAC_ADDRESS) {
    this->sendToAllInterfaces(frame);
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

void switch_protocol::sendToAllInterfaces(link::Frame& frame) {
  logger.info("Bradcast MAC sent throw all interfaces");
  
  ushort source_interface = interface_amount; // invalid interface;
  if (forwarding_table.find(frame.MAC_source) != forwarding_table.end()) {
    source_interface = forwarding_table.at(frame.MAC_source);
  }

  for(ushort i=0; i<interface_amount; ++i) {
    if (i != source_interface) {
      this->send(frame,i);
    }
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