#if !defined dns_server_protocol_h
#define dns_server_protocol_h

#include "layer.h"

/* Documentation
 *
 * 
 */

class dns_server_protocol: public Layer<int, int, dns::Packet, udp::Control> { 

protected:

  bool recursive;
  bool starting;
  IPv4 server_ip;
  std::list<dns::ResourceRecord> dns_table;

  /********** TIMES ***************/
  double process_dns_query = 0.001;

  /********* Protected methods *********/


public:
	dns_server_protocol(const char *n): Layer(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();

  virtual void dinternal(double);
  virtual void dexternal(double) {};
};

#endif