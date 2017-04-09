//#include <iostream>

//#include "abstract_types.h"
//#include "socket.h"
//#include "ipv4.h"
#include "udp.h"
//#include "app.h"
//#include "ip.h"
//#include "link.h"
//#include "swp.h"
#include "dns.h"

#include "../libs/parser.h"
using namespace std;

/*
bool swpInWindow(swp::Seqno startNum, ushort WS, swp::Seqno seqNum) {
  for (swp::Seqno i=0; i<WS; ++i) {
  	swp::Seqno current = startNum+i;
  	if (seqNum == current)
      return true;
  }
  return false;
}

void load_swp_hdr(swp::Hdr& hdr, const unsigned long& preamble) {
  unsigned long stored_hdr = 0x00000000;
  unsigned long curr_val = 0x00000000;

  stored_hdr = 0x00F00000 & preamble;
  cout << stored_hdr << endl;
  stored_hdr = stored_hdr >> 20;
  cout << stored_hdr << endl;
  hdr.Flags = stored_hdr;

  stored_hdr = 0x0F000000 & preamble;
  cout << stored_hdr << endl;
  stored_hdr = stored_hdr >> 24;
  cout << stored_hdr << endl;
  hdr.AckNum = stored_hdr;

  stored_hdr = 0xF0000000 & preamble;
  cout << stored_hdr << endl;
  stored_hdr = stored_hdr >> 28;
  cout << stored_hdr << endl;
  hdr.SeqNum = stored_hdr;
}
*/

int main() {

	Parser<dns::Packet> p("test.txt");
	cout << p.next_input() << endl;
	cout << p.next_input() << endl;

	/*

	dns::Packet p;
	cin >> p;
	cout << p << endl;
	
	dns::Header b;

	b.id = 0xFF;
	b.flags_code = 0xFE;
	b.QDCount = 0xFF;
	b.ANCount = 0;
	b.NSCount = 2;
	b.ARCount = 1;

	const char* d = b.c_str();
	dns::Header e(d);

	std::cout << b << std::endl << std::endl;
	std::cout << e << std::endl << std::endl;

	dns::ResourceRecord a;

	a.name = std::string("el-atajo.com.ar");
	a.QType = dns::Type::A;
	a.AValue = "1.0.0.2";
	a.QClass = 0xFFFF;
	a.TTL = 0xFFFF;

	const char* h = NULL;
	h = a.c_str();

	dns::ResourceRecord g = a;
	dns::ResourceRecord f(h);

	std::cout << hex;
	std::cout << a << std::endl;
	std::cout << f << std::endl;
	std::cout << g << std::endl;

	delete[] d;
	delete[] h;

	dns::ResourceRecord dn;
	std::cin >> dn;
	std::cout << dn << std::endl;
	*/
	/*
	
	dns::Packet p;

	p.header.id = 0xFF;
	p.header.flags_code = 0xFE;
	p.header.QDCount = 1;
	p.header.ANCount = 0;
	p.header.NSCount = 1;
	p.header.ARCount = 1;

	dns::ResourceRecord a;

	a.name = std::string("el-atajo.com.ar");
	a.QType = dns::Type::A;
	a.AValue = "1.0.0.2";
	a.QClass = 0xFFFF;
	a.TTL = 0xFFFF;


	dns::ResourceRecord b;

	b.name = std::string("andaen.com");
	b.QType = dns::Type::A;
	b.AValue = "1.0.0.2";
	b.QClass = 0xFFFF;
	b.TTL = 0xFFFF;

	dns::ResourceRecord c;

	c.name = std::string("el-atajo.com.ar");
	c.QType = dns::Type::NS;
	c.NSValue = std::string("dns.server.ar");
	c.QClass = 0xFFFF;
	c.TTL = 0xFFFF;

	dns::ResourceRecord d;

	d.name = std::string("el-atajo.com.ar");
	d.QType = dns::Type::NS;
	d.NSValue = std::string("hola.test.como.estas.esto.es.muy.largo");
	d.QClass = 0xFFFF;
	d.TTL = 0xFFFF;

	p.questions.push_back(a);
	p.authoritatives.push_back(b);
	p.authoritatives.push_back(c);
	p.aditionals.push_back(d);

	cout << p << endl;
	cout << "size of dns::Packet p: " << p.size() << endl;

	const char* p_str = p.c_str();
	dns::Packet p2(p_str);

	cout << p2 << endl;

	delete[] p_str;


	cout << "size of dns::Packet p2: " << p2.size() << endl;
	*/
	/*
	swp::Seqno a = 250;

	for (int i = 0; i < 20; ++i) {
		cout << (int)a << endl;
		++a;
	}
	*/

	/*
	swp::Seqno seqNum[4] = {8,9,254,255};
	std::list<swp::recvQ_slot<link::Frame>> recvQ(4, swp::recvQ_slot<link::Frame>());
	int i = 0;
	for (std::list<swp::recvQ_slot<link::Frame>>::iterator it = recvQ.begin(); it != recvQ.end(); ++it){
		it->SeqNum = seqNum[i];
		++i;
	}

	for (std::list<swp::recvQ_slot<link::Frame>>::iterator it = recvQ.begin(); it != recvQ.end(); ++it){
		cout << (int)it->SeqNum << " ";
	}
	cout << endl;

	recvQ.sort(swp::recvQ_slotComparator<link::Frame>);

	for (std::list<swp::recvQ_slot<link::Frame>>::iterator it = recvQ.begin(); it != recvQ.end(); ++it){
		cout << (int)it->SeqNum << " ";
	}
	cout << endl;

	swp::recvQ_slot<link::Frame> r;
	r.SeqNum = 7;
	recvQ.push_back(r);

	for (std::list<swp::recvQ_slot<link::Frame>>::iterator it = recvQ.begin(); it != recvQ.end(); ++it){
		cout << (int)it->SeqNum << " ";
	}
	cout << endl;

	recvQ.sort(swp::recvQ_slotComparator<link::Frame>);

	for (std::list<swp::recvQ_slot<link::Frame>>::iterator it = recvQ.begin(); it != recvQ.end(); ++it){
		cout << (int)it->SeqNum << " ";
	}
	cout << endl;
	*/

	/*
	cout << RWS << endl;
	swp::Seqno a = 10;
	swp::Seqno seqNum[10] = {8,9,10,11,12,13,14,15,16,17};
	for(int i=0; i<10; ++i) {
		cout << "seqNum " << (int)seqNum[i] << ": ";
		cout << (swp::SeqnoRWSComparator(a,seqNum[i]) ? "<": ">=") << endl;
	}
	*/

	/*
	swp::Seqno startNum = 10;
	ushort ws = 4;
	swp::Seqno seqNum[10] = {8,9,10,11,12,13,14,15,16,17};
	
	for(int i=0; i<10; ++i) {
		cout << "seqNum " << (int)seqNum[i] << ": ";
		cout << (swpInWindow(startNum,ws,seqNum[i]) ? "True": "False") << endl;
	} */

	/*
	swp::Seqno startNum = 0xFE;
	ushort ws = 4;
	swp::Seqno seqNum[10] = {0xFC,0xFD,0xFE,0xFF,0,1,2,3,4,5};
	
	for(int i=0; i<10; ++i) {
		cout << "seqNum " << (int)seqNum[i] << ": ";
		cout << (swpInWindow(startNum,ws,seqNum[i]) ? "True": "False") << endl;
	}
	*/
	/*
	cout << hex;
	unsigned long preamble = 0xabc01010;
	swp::Hdr hdr;
	hdr.SeqNum = 0x1;
	hdr.AckNum = 0x1;
	hdr.Flags = 0x1;
	cout << "preamble: " << preamble << endl;
	cout << "hdr: " << endl << hdr << endl;
	load_swp_hdr(hdr, preamble);
	cout << "preamble: " << preamble << endl;
	cout << "hdr: " << endl << hdr << endl;
	cout << dec;
	*/
	/*
	ip::Datagram ip_packet;
	ip_packet.header.vide = 1;
	ip_packet.header.total_length = 2;
	ip_packet.header.identification = 3;
	ip_packet.header.ff = 4;
	ip_packet.header.ttlp = 5;
	ip_packet.header.header_checksum = 6;
	ip_packet.header.src_ip = "127.0.0.1";
	ip_packet.header.dest_ip = "127.0.0.2";

	ip_packet.data.psd_header.src_ip = "127.0.0.3";
	ip_packet.data.psd_header.dest_ip = "127.0.0.4";
	ip_packet.data.psd_header.zeros = 7;
	ip_packet.data.psd_header.protocol = 8;
	ip_packet.data.psd_header.udp_lenght = 9;

	ip_packet.data.header.src_port = 10;
	ip_packet.data.header.dest_port = 11;
	ip_packet.data.header.length = 12;
	ip_packet.data.header.checksum = 13;
	
	memcpy(ip_packet.data.payload,"hello world",12);

	link::Frame frame;
	frame.MAC_destination = "0:0:0:0:0:1";
	frame.MAC_source = "0:0:0:0:0:2";
	frame.EtherType = 1;
	frame.CRC = 1;
	frame.setPayload(ip_packet);

	cout << frame << endl;

	ip::arp::Packet arp_packet;

	arp_packet.Hardware_type = 14;
	arp_packet.Protocol_type = 15;
	arp_packet.HLen = 16;
	arp_packet.PLen = 17;
	arp_packet.Operation = 18;
	arp_packet.Source_Hardware_Address = "0:0:0:0:0:3";
	arp_packet.Source_Protocol_Address = "127.0.0.5";
	arp_packet.Target_Hardware_Address = "0:0:0:0:0:4";
	arp_packet.Target_Protocol_Address = "127.0.0.6";

	link::Frame arp_frame;
	arp_frame.MAC_destination = "0:0:0:0:0:3";
	arp_frame.MAC_source = "0:0:0:0:0:4";
	arp_frame.EtherType = 2;
	arp_frame.CRC = 2;
	arp_frame.setPayload(arp_packet);

	cout << arp_frame << endl;
	*/
	return 0;
}