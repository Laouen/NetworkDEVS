#if !defined ipv4_h
#define ipv4_h

#include <cstdint> // Allows using ushort
#include <fstream>
#include <cstring> // memcpy

struct IPv4 {
	ushort* ip;

  IPv4() {
    ip = new ushort[4];
  }
  IPv4(ushort* other_ip) {
    ip = new ushort[4];
    std::memcpy(ip,other_ip,4);
  }

  IPv4(const IPv4& other) {
    ip = new ushort[4];
    std::memcpy(ip,other.ip,4);
  }

  bool operator==(const IPv4& other) {
    bool res = true;
    for(int i=0; i<4; ++i)
      res = res && ip[i] == other.ip[i];
    return res;
  }

  bool operator<(const IPv4& other) {
    for(int i=0; i<4; ++i) {
      if (ip[i] < other.ip[i])
        return true;
      else if (ip[i] > other.ip[i])
        return false;
    }
    return false;
  }

  friend std::ostream& operator<<(std::ostream&, const IPv4&);
  friend std::istream& operator>>(std::istream&, const IPv4&);
};

inline std::ostream& operator<<(std::ostream& os, const IPv4& ip) {
  os << ip.ip[0] << "." << ip.ip[1] << "." << ip.ip[2] << "." << ip.ip[3];
  return os;
}

inline std::istream& operator>>(std::istream& is, IPv4& ip) {
  ip.ip = new ushort[4];
  char ch;
  is >> ip.ip[0] >> ch >> ip.ip[1] >> ch >> ip.ip[2] >> ch >> ip.ip[3];
  return is;
}

#endif