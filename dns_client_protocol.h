#if !defined dns_client_protocol_h
#define dns_client_protocol_h

#include "layer.h"

/* Documentation
 *
 * 
 */

class dns_clienter_protocol: public Layer<std::string, int, dns::Packet, udp::Control> { 

protected:

  bool starting;
  IPv4 client_ip;
  IPv4 local_server_ip;

  /********** TIMES ***************/
  double process_dns_query = 0.001;

  /********* Protected methods *********/
  void processDNSQuery(std::string);
  void processDNSResponse(dns::Packet);

public:
	dns_clienter_protocol(const char *n): Layer(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();

  virtual void dinternal(double);
  virtual void dexternal(double) {};
};

#endif