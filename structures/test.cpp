#include <iostream>

#include "abstract_types.h"
#include "socket.h"
#include "ipv4.h"
#include "udp.h"
#include "app.h"
#include "ip.h"
#include "link.h"
#include "swp.h"

using namespace std;

bool swpInWindow(swp::SwpSeqno startNum, ushort WS, swp::SwpSeqno seqNum) {
  for (swp::SwpSeqno i=0; i<WS; ++i) {
  	swp::SwpSeqno current = startNum+i;
  	if (seqNum == current)
      return true;
  }
  return false;
}

void load_swp_hdr(swp::SwpHdr& hdr, const unsigned long& preamble) {
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

int main() {

	swp::SwpSeqno a = 250;

	for (int i = 0; i < 20; ++i) {
		cout << (int)a << endl;
		++a;
	}

	/*
	swp::SwpSeqno seqNum[4] = {8,9,254,255};
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
	swp::SwpSeqno a = 10;
	swp::SwpSeqno seqNum[10] = {8,9,10,11,12,13,14,15,16,17};
	for(int i=0; i<10; ++i) {
		cout << "seqNum " << (int)seqNum[i] << ": ";
		cout << (swp::SeqnoRWSComparator(a,seqNum[i]) ? "<": ">=") << endl;
	}
	*/

	/*
	swp::SwpSeqno startNum = 10;
	ushort ws = 4;
	swp::SwpSeqno seqNum[10] = {8,9,10,11,12,13,14,15,16,17};
	
	for(int i=0; i<10; ++i) {
		cout << "seqNum " << (int)seqNum[i] << ": ";
		cout << (swpInWindow(startNum,ws,seqNum[i]) ? "True": "False") << endl;
	} */

	/*
	swp::SwpSeqno startNum = 0xFE;
	ushort ws = 4;
	swp::SwpSeqno seqNum[10] = {0xFC,0xFD,0xFE,0xFF,0,1,2,3,4,5};
	
	for(int i=0; i<10; ++i) {
		cout << "seqNum " << (int)seqNum[i] << ": ";
		cout << (swpInWindow(startNum,ws,seqNum[i]) ? "True": "False") << endl;
	}
	*/
	/*
	cout << hex;
	unsigned long preamble = 0xabc01010;
	swp::SwpHdr hdr;
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
	ip::Packet ip_packet;
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