#if !defined dns_h
#define dns_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

#include "abstract_types.h"

#define IS_AUTHORITATIVE_ANSWER
#define IS_TRUNCATED
#define RECURSION_DESIRED
#define RECURSION_AVAILABLE
#define RECURSION_AVAILABLE

// Application layer structures
namespace dns {

  struct Header : abstract::Header {
    ushort id;
    ushort flags_code;
    ushort QDCount;
    ushort ANCount;
    ushort NSCount;
    ushort ARCount;

    Header() {}
    Header(const Header& other) {
      id = other.id;
      flags_code = other.flags_code;
      QDCount = other.QDCount;
      ANCount = other.ANCount;
      NSCount = other.NSCount;
      ARCount = other.ARCount;
    }
    Header(const char* other) {
      memcpy(&id,other,2);
      memcpy(&flags_code,&other[2],2);
      memcpy(&QDCount,&other[4],2);
      memcpy(&ANCount,&other[6],2);
      memcpy(&NSCount,&other[8],2);
      memcpy(&ARCount,&other[10],2);
    }

    ushort size() const {
      return 12;
    }

    const char* c_str() const {
      char* res = new char[this->size()];

      memcpy(res,&id,2);
      memcpy(&res[2],&flags_code,2);
      memcpy(&res[4],&QDCount,2);
      memcpy(&res[6],&ANCount,2);
      memcpy(&res[8],&NSCount,2);
      memcpy(&res[10],&ARCount,2);
      return res;
    }
  };

  struct DN {
    std::list<std::string> name;

    DN() {};
    DN(const DN& other) {
      name = other.name;
    }
    DN(std::string str) {
      std::stringstream ss(str);
      std::string sub_domain;

      while(getline(ss, sub_domain, '.')) {
        name.push_back(sub_domain);
      }
    }
    DN(const char* data) {
      int i = 0;
      while (data[i] != 0x00) { /* data[i] == 0 */
        int len = data[i];
        name.push_back(std::string(&data[i+1], len));
        i += len + 1;
      }
    }

    const char* c_str() const {
      char* res = new char[this->size()];
      for (int i = 0; i < this->size(); ++i) res[i] = 0x00; // last 0x0 character is initialized here
      int len;
      int i = 0;
      std::list<std::string>::const_iterator it;
      for (it = name.begin(); it != name.end(); ++it) {
        len = it->length();
        res[i] = (unsigned char)len;
        memcpy(&res[i+1],it->c_str(),len);
        i += len + 1;
      }
      return res;
    }

    int size() const {
      int size = 0;
      std::list<std::string>::const_iterator it;
      for (it = name.begin(); it != name.end(); ++it) {
        size += it->length() + 1;
      }
      return size+1; // last 0 char value
    }
  };

  // Resource Record
  struct RR {
    DN name;
    ushort QType;
    ushort QClass;
    ushort TTL;

    RR() {}
    RR(const RR& other) {
      name = other.name;
      QType = other.QType;
      QClass = other.QClass;
      TTL = other.TTL;
    }
    RR(const char *data) {
      name = DN(data);
      int name_size = name.size();
      memcpy(&QType,  &data[name_size],   2);
      memcpy(&QClass, &data[name_size+2], 2);
      memcpy(&TTL,    &data[name_size+4], 2);
    }

    const char* c_str() const {
      char* res = new char[this->size()];
      for (int i = 0; i < this->size(); ++i) res[i] = 0x00;
      const char* name_str = name.c_str();

      int name_size = name.size();
      
      memcpy(res,name_str,name_size);
      memcpy(&res[name_size],&QType,2);
      memcpy(&res[name_size+2],&QClass,2);
      memcpy(&res[name_size+4],&TTL,2);

      delete[] name_str;
      return res;
    }

    int size() const {
      return name.size() + 6;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, dns::DN& d) {
  std::list<std::string>::const_iterator it = d.name.begin();
  while (it != d.name.end()) {
    os << *it;
    ++it;
    if (it != d.name.end()) os << '.';
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, dns::Header& h) {
  os << "id: " << h.id << std::endl;
  os << "flags_code: " << h.flags_code << std::endl;
  os << "QDCount: " << h.QDCount << std::endl;
  os << "ANCount: " << h.ANCount << std::endl;
  os << "NSCount: " << h.NSCount << std::endl;
  os << "ARCount: " << h.ARCount;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, dns::RR& r) {
  os << "name: " << r.name << std::endl;
  os << "QType: " << r.QType << std::endl;
  os << "QClass: " << r.QClass << std::endl;
  os << "TTL: " << r.TTL << std::endl;
  return os;
}

#endif