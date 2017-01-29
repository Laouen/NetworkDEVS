#if !defined mac_h
#define mac_h

#include <cstdint> // Allows using ushort
#include <fstream>
#include <string>
#include <cstring> // memcpy
#include <sstream>
#include <iostream>

#define BROADCAST_MAC_ADDRESS std::string("FF:FF:FF:FF:FF:FF")

struct MAC {
	ushort addr[6];

  MAC() {
    for(int i=0; i<6; ++i)
      addr[i] = 0;
  }

  MAC(const std::string& other_mac) {
    char ch;
    std::istringstream strm(other_mac);
    strm >> std::hex >> addr[0] >> ch >> std::hex >> addr[1] >> ch >> std::hex >> addr[2] >> ch; 
    strm >> std::hex >> addr[3] >> ch >> std::hex >> addr[4] >> ch >> std::hex >> addr[5];
  }

  MAC(const char* other_mac) {
    char ch;
    std::istringstream strm(other_mac);
    strm >> std::hex >> addr[0] >> ch >> std::hex >> addr[1] >> ch >> std::hex >> addr[2] >> ch; 
    strm >> std::hex >> addr[3] >> ch >> std::hex >> addr[4] >> ch >> std::hex >> addr[5];
  }

  MAC(ushort* other_mac) {
    for(int i=0; i<6; ++i)
      addr[i] = other_mac[i];
  }

  MAC(const MAC& other_mac) {
    for(int i=0; i<6; ++i)
      addr[i] = other_mac.addr[i];
  }

  bool operator==(const MAC& other) const {
    bool res = true;
    for(int i=0; i<6; ++i)
      res = res && addr[i] == other.addr[i];
    return res;
  }

  bool operator==(const std::string& other_string) const {
    MAC other = other_string;
    return *this == other;
  }

  bool operator<(const MAC& other) const {
    for(int i=0; i<6; ++i) {
      if (addr[i] < other.addr[i])
        return true;
      else if (addr[i] > other.addr[i])
        return false;
    }
    return false;
  }

  bool operator<(const std::string& other_string) const {
    MAC other = other_string;
    return *this < other; 
  }

  std::string as_string() const {
    std::stringstream ss;
    for (int i=0; i<6; ++i) {
      ss << std::hex << addr[i];
      if(i<5) ss << ":";
    }
    return ss.str();
  }

  MAC operator&(const MAC& other) const {
    MAC res;
    for(int i=0; i<6; ++i)
      res.addr[i] = addr[i] & other.addr[i];
    return res;
  }

  MAC operator|(const MAC& other) const {
    MAC res;
    for(int i=0; i<6; ++i)
      res.addr[i] = addr[i] | other.addr[i];
    return res;
  }
};

inline std::ostream& operator<<(std::ostream& os, const MAC& mac) {
  for (int i=0; i<6; ++i) {
    os << std::hex << mac.addr[i];
    if(i<5) os << ":";
  }
  os << std::dec;
  return os;
}

inline std::istream& operator>>(std::istream& is, MAC& mac) {
  char ch;
  is >> std::hex >> mac.addr[0] >> ch >> std::hex >> mac.addr[1] >> ch >> std::hex >> mac.addr[2] >> ch; 
  is >> std::hex >> mac.addr[3] >> ch >> std::hex >> mac.addr[4] >> ch >> std::hex >> mac.addr[5];
  return is;
}

#endif