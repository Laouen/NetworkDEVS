#include "ipv4.h"
#include "ip.h"

#include <iostream>

using namespace std;

void calculateChecksum(const char* header, ushort count) {

  ushort* h = (ushort*)header;
  ushort vide = h[0];
  ushort total_length = h[1];
  ushort identification = h[2];
  ushort ff = h[3];
  ushort ttlp = h[4];
  IPv4 dest_ip(&h[5]);
  IPv4 src_ip(&h[9]);

  cout << "vide: " << vide << endl;
  cout << "total_length: " << total_length << endl;
  cout << "identification: " << identification << endl;
  cout << "ff: " << ff << endl;
  cout << "ttlp: " << ttlp << endl;
  cout << "dest_ip: " << dest_ip.as_string() << endl;
  cout << "src_ip: " << src_ip.as_string() << endl;
}

const char* transform(const ip::Header& h) {
  char* foo = new char[25];
  memcpy(foo, &h.vide, 2);
  memcpy(&foo[2], &h.total_length, 2);
  memcpy(&foo[4], &h.identification, 2);
  memcpy(&foo[6], &h.ff, 2);
  memcpy(&foo[8], &h.ttlp, 2);
  memcpy(&foo[10], h.dest_ip.ip, 8);
  memcpy(&foo[18], h.src_ip.ip, 8);
  return foo;
}

int main() {

  ip::Header h;
  h.vide = 255;
  h.total_length = 254;
  h.identification = 253;
  h.ff = 252;
  h.ttlp = 251;
  h.header_checksum = 250;
  h.src_ip = "140.0.0.10";
  h.dest_ip = "150.5.5.5";

  calculateChecksum(transform(h), 8);
  return 0;
}