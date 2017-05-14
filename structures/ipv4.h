#if !defined ipv4_h
#define ipv4_h

#include <cstdint> // Allows using ushort
#include <fstream>
#include <string>
#include <cstring> // memcpy
#include <sstream>
#include <iostream>

/**
 * @author Laouen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @struct IPv4 ipv4.h
 * @brief This class declare a IPV4 type for NetworkDEVS simulation purposes. 
 *
 * @details It's not meant to be a full IPv4 class, for that purpose, there is many 
 * external C++ libraries. This class was designed to easely simulate Networks 
 * using the NetworkDEVS framework.
 *
 */
struct IPv4 {
	ushort ip[4];

  IPv4() {
    for(int i=0; i<4; ++i)
      ip[i] = 0;
  }

  IPv4(const std::string& other_ip) {
    char ch;
    std::istringstream strm(other_ip);
    strm >> ip[0] >> ch >> ip[1] >> ch >> ip[2] >> ch >> ip[3];
  }

  IPv4(const char* other_ip) {
    char ch;
    std::istringstream strm(other_ip);
    strm >> ip[0] >> ch >> ip[1] >> ch >> ip[2] >> ch >> ip[3];
  }

  IPv4(ushort* other_ip) {
    for(int i=0; i<4; ++i)
      ip[i] = other_ip[i];
  }

  IPv4(const IPv4& other_ip) {
    for(int i=0; i<4; ++i)
      ip[i] = other_ip.ip[i];
  }

  bool operator==(const IPv4& other) const {
    bool res = true;
    for(int i=0; i<4; ++i)
      res = res && ip[i] == other.ip[i];
    return res;
  }

  bool operator<(const IPv4& other) const {
    for(int i=0; i<4; ++i) {
      if (ip[i] < other.ip[i])
        return true;
      else if (ip[i] > other.ip[i])
        return false;
    }
    return false;
  }

  std::string as_string() const {
    std::string res = "";
    for (int i = 0; i < 4; ++i) {
      res += std::to_string(ip[i]);
      if(i<3) res += ".";
    }
    return res;
  }

  IPv4 operator&(const IPv4& other) const {
    IPv4 res;
    for(int i=0; i<4; ++i)
      res.ip[i] = ip[i] & other.ip[i];
    return res;
  }

  IPv4 operator|(const IPv4& other) const {
    IPv4 res;
    for(int i=0; i<4; ++i)
      res.ip[i] = ip[i] | other.ip[i];
    return res;
  }

  int size() const {
    return 4 * sizeof(ip[0]);
  }


  friend std::ostream& operator<<(std::ostream&, const IPv4&);
  friend std::istream& operator>>(std::istream&, IPv4&);
};

inline std::ostream& operator<<(std::ostream& os, const IPv4& ip) {
  os << ip.ip[0] << "." << ip.ip[1] << "." << ip.ip[2] << "." << ip.ip[3];
  return os;
}

inline std::istream& operator>>(std::istream& is, IPv4& ip) {
  char ch;
  is >> ip.ip[0] >> ch >> ip.ip[1] >> ch >> ip.ip[2] >> ch >> ip.ip[3];
  return is;
}

#endif