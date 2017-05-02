#include "switch_protocol.h"

void switch_protocol::init(double t, ...) {
  
  // PowerDEVS parameters
  va_list parameters;
  va_start(parameters,t);

  // Set logger module name
  std::string module_name = va_arg(parameters,char*);
  logger.setModuleName("Switch " + module_name);

  // reading mac
  mac = va_arg(parameters, char*);
  logger.info("MAC: " + mac.as_string());

  interface_amount = (ushort)va_arg(parameters,double);
  logger.info("Interface amounts: " + std::to_string(interface_amount));

  // initializing swp ports
  for (int i=0; i<interface_amount; ++i) {
    logger.info("Added swp protocol for port " + std::to_string(i));
    swp_ports.push_back(swp_protocol());
    swp_ports.back().init(mac, module_name + " Port: " + std::to_string(i));
  }

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
  std::vector<swp_protocol>::iterator it;

  for(it=swp_ports.begin(); it!=swp_ports.end(); ++it) {
    next_internal = std::min(next_internal, it->nexTimeout());
  }
  return next_internal;
}

Event switch_protocol::lambda(double t) {

  return output;
}

void switch_protocol::dinternal(double t) {

  this->updateTimeouts(t);

  bool frames_sent = false;
  for (ushort i=0; i<swp_ports.size(); ++i) {
    if (!swp_ports[i].to_send_frames.empty()) {
      this->sendFrames(i);
      frames_sent = true;
    }
  }

  if (frames_sent) {
    next_internal = send_frame_time;
    return;
  }

  for (ushort i=0; i<swp_ports.size(); ++i) {
    if (swp_ports[i].thereIsFrameToSend()) {
      swp_ports[i].send();
      next_internal = swp_ports[i].send_time;
      return;
    }
  }

  for (ushort i=0; i<swp_ports.size(); ++i) {
    if (!swp_ports[i].accepted_frames.empty()) {
      link::Frame frame = swp_ports[i].accepted_frames.front();
      this->processFrame(frame,i);
      swp_ports[i].accepted_frames.pop();
      next_internal = process_frame_time;
      return;
    }
  }

  for (ushort i=0; i<swp_ports.size(); ++i) {
    if (swp_ports[i].timeoutTriggered()) {
      swp_ports[i].timeout();
      next_internal = swp_timeout_time;
      return;
    }
  }

  if (!lower_layer_data_in.empty()) {
    link::Multiplexed_frame mf = lower_layer_data_in.front();
    swp_ports[mf.interface].processFrame(mf.frame);
    lower_layer_data_in.pop();
    next_internal = process_frame_time;
    return;
  }
}

void switch_protocol::dexternal(double t) {

  this->updateTimeouts(t);
}

/***************** Datagram **************************/

void switch_protocol::processFrame(link::Frame& frame, ushort interface) {
  logger.debug("switch_protocol::processFrame");
  if (frame.MAC_destination == BROADCAST_MAC_ADDRESS) {
    this->sendToAllInterfaces(frame, interface);
    return;
  }

  if (forwarding_table.find(frame.MAC_destination) == forwarding_table.end()) {
    // this model does not implement any dinamic method to get the interface
    logger.info("MAC_destination " + 
                frame.MAC_destination.as_string() + 
                " not found in the forwarding table");
    return;
  }

  this->send(frame, forwarding_table.at(frame.MAC_destination));
}

void switch_protocol::sendFrames(ushort interface) {
  logger.debug("switch_protocol::sendFrames");
  link::Multiplexed_frame multiplexed_frame;

  while(!swp_ports[interface].to_send_frames.empty()) {
    multiplexed_frame.frame = swp_ports[interface].to_send_frames.front();
    multiplexed_frame.frame.CRC = this->calculateCRC();
    multiplexed_frame.interface = interface;
    lower_layer_data_out.push(multiplexed_frame);
    swp_ports[interface].to_send_frames.pop();
  }
}

void switch_protocol::sendToAllInterfaces(link::Frame& frame, ushort source_interface) {
  logger.info("Bradcast MAC sent throw all interfaces");
  logger.debug("source interface: " + std::to_string(source_interface));

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

  swp_ports[interface].sendFrame(frame);
}

void switch_protocol::updateTimeouts(double t) {
  for (int i=0; i<swp_ports.size(); ++i) {
    swp_ports[i].updateTimeouts(t-last_transition);
  }
}

/*********** Unimplemented Comunication Methods *****************/

unsigned long switch_protocol::calculateCRC() { return 0; }

bool switch_protocol::verifyCRC(link::Frame frame) { return true; }