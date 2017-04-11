#if !defined dns_server_h
#define dns_server_h

#include "layer.h"

/* Documentation
 *
 * 
 */

class dns_server: public Layer<dns::DomainName, int, dns::Packet, udp::Control, dns::Packet> { 

protected:

  int next_id;
  bool bind;
  IPv4 client_ip;
  IPv4 local_root_server_ip;
  std::list<dns::ResourseRecord> chaced_RRs;
  std::list<dns::ResourseRecord> authoritative_RRs;
  std::list<dns::Packet> app_requests;
  std::list<dns::Packet> host_requests;

  /********** TIMES ***************/
  double process_dns_query = 0.001;
  double process_dns_response = 0.001;

  /********* Protected methods *********/

public:
	dns_server(const char *n): Layer(n) {};
	void init(double, ...);
	double ta(double t);
  Event lambda(double);
  void exit();

  virtual void dinternal(double);
  virtual void dexternal(double);
};

#endif