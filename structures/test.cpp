#include "ipv4.h"
#include "udp.h"

#include <iostream>

using namespace std;

void printChecksum(const char* header) {

  ushort* h = (ushort*)header;
  unsigned char* h2 = (unsigned char*)header;
  ushort src_port = h[0];
  ushort dest_port = h[1];
  ushort length = h[2];
  IPv4 src_ip(&h[3]);
  IPv4 dest_ip(&h[7]);
  ushort udp_lenght = h[11];
  ushort protocol = h2[24];
  ushort zeros = h2[25];

  cout << "src_port: " << src_port << endl;
  cout << "dest_port: " << dest_port << endl;
  cout << "length: " << length << endl;
  cout << "src_ip: " << src_ip << endl;
  cout << "dest_ip: " << dest_ip << endl;
  cout << "udp_lenght: " << udp_lenght << endl;
  printf("%s%i\n","zeros: ",zeros);
  printf("%s%i\n","protocol: ",protocol);
}

void printHeader(const char* header) {

  ushort* h = (ushort*)header;
  ushort src_port = h[0];
  ushort dest_port = h[1];
  ushort length = h[2];

  cout << "src_port: " << src_port << endl;
  cout << "dest_port: " << dest_port << endl;
  cout << "length: " << length << endl;
}

void printPseudoheader(const char* pseudoheader) {
  ushort* h = (ushort*)pseudoheader;
  unsigned char* h2 = (unsigned char*)pseudoheader;
  IPv4 src_ip(&h[0]);
  IPv4 dest_ip(&h[4]);
  ushort udp_lenght = h[8];
  unsigned char zeros = h2[18];
  unsigned char protocol = h2[19];

  cout << "src_ip: " << src_ip << endl;
  cout << "dest_ip: " << dest_ip << endl;
  cout << "udp_lenght: " << udp_lenght << endl;
  printf("%s%i\n","zeros: ",zeros);
  printf("%s%i\n","protocol: ",protocol);
}

int main() {

  udp::Datagram d;
  d.header.src_port = 80;
  d.header.dest_port = 8080;
  d.header.length = 15;
  d.psd_header.src_ip = "150.5.5.10";
  d.psd_header.dest_ip = "140.0.0.10";
  d.psd_header.udp_lenght = 8;
  d.psd_header.protocol = 0xF;
  d.psd_header.zeros = 0xE;
  printChecksum(d.headers_c_str());
  
  udp::Header h;
  h.src_port = 80;
  h.dest_port = 8080;
  h.length = 15;
  printHeader(h.c_str());

  udp::PseudoHeader ph;
  ph.src_ip = "150.5.5.10";
  ph.dest_ip = "140.0.0.10";
  ph.udp_lenght = 8;
  ph.zeros = 0xF;
  ph.protocol = 0xF;
  printPseudoheader(ph.c_str());
  return 0;
}