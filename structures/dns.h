#if !defined dns_h
#define dns_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <cstring> // memcpy
#include <list>
#include <sstream> // istringstring
#include <iostream>

#include <string>
#include <fstream>

#include "abstract_types.h"
#include "ipv4.h"

#define IS_AUTHORITATIVE_ANSWER
#define IS_TRUNCATED
#define RECURSION_DESIRED
#define RECURSION_AVAILABLE
#define RECURSION_AVAILABLE

// Application layer structures
namespace dns {

  enum Type : uint16_t {A = 1, NS = 2 };

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
      std::memcpy(&id,other,2);
      std::memcpy(&flags_code,&other[2],2);
      std::memcpy(&QDCount,&other[4],2);
      std::memcpy(&ANCount,&other[6],2);
      std::memcpy(&NSCount,&other[8],2);
      std::memcpy(&ARCount,&other[10],2);
    }

    ushort size() const {
      return 12;
    }

    const char* c_str() const {
      char* res = new char[this->size()];

      std::memcpy(res,&id,2);
      std::memcpy(&res[2],&flags_code,2);
      std::memcpy(&res[4],&QDCount,2);
      std::memcpy(&res[6],&ANCount,2);
      std::memcpy(&res[8],&NSCount,2);
      std::memcpy(&res[10],&ARCount,2);
      return res;
    }
  };

  struct DomainName {
    std::list<std::string> name;

    DomainName() {};
    DomainName(const DomainName& other) {
      name = other.name;
    }
    DomainName(std::string str) {
      std::stringstream ss(str);
      std::string sub_domain;

      while(getline(ss, sub_domain, '.')) {
        name.push_back(sub_domain);
      }
    }
    DomainName(const char* data) {
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
        std::memcpy(&res[i+1],it->c_str(),len);
        i += len + 1;
      }
      return res;
    }

    std::string as_string() {
      std::string res = "";
      std::list<std::string>::const_iterator it = name.begin();
      while (it != name.end()) {
        res += *it;
        ++it;
        if (it != name.end()) res += '.';
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
  struct ResourceRecord {
    DomainName name;
    Type QType;
    IPv4 AValue;
    DomainName NSValue;
    ushort QClass;
    ushort TTL;

    ResourceRecord() {}
    ResourceRecord(const ResourceRecord& other) {
      name = other.name;
      AValue = other.AValue;
      NSValue = other.NSValue;
      QType = other.QType;
      QClass = other.QClass;
      TTL = other.TTL;
    }
    ResourceRecord(const char *data) {
      
      // Reading name
      name = DomainName(data);
      int i = name.size();
      
      // Reading type
      std::memcpy(&QType, &data[i], sizeof(QType));
      i += sizeof(QType);

      // Reading value
      if (QType == Type::A) {
        AValue = IPv4(&data[i]);
        i += AValue.size();
      } else if (QType == Type::NS) {
        NSValue = DomainName(&data[i]);
        i += NSValue.size();
      }

      //Reading class
      std::memcpy(&QClass, &data[i], sizeof(QClass));
      i += sizeof(QClass);

      // Reading TTL
      std::memcpy(&TTL, &data[i+4], sizeof(TTL));
    }

    const char* c_str() const {
      char* res = new char[this->size()];
      for (int i = 0; i < this->size(); ++i) res[i] = 0x00;
      const char* name_str = name.c_str();

      int name_size = name.size();
      
      std::memcpy(res,name_str,name_size);
      std::memcpy(&res[name_size],&QType,2);
      std::memcpy(&res[name_size+2],&QClass,2);
      std::memcpy(&res[name_size+4],&TTL,2);

      delete[] name_str;
      return res;
    }

    int size() const {
      return name.size() + 6;
    }

    std::string as_string() {
      std::string res = "";
      res += this->name.as_string() + " ";
      switch(this->QType) {
      case dns::Type::A: res += "A "; res += this->AValue.as_string() + " "; break;
      case dns::Type::NS: res += "NS "; res += this->NSValue.as_string() + " "; break;
      }
      res += std::to_string(this->QClass) + " ";
      res += std::to_string(this->TTL) + " ";
      return res;
    }
  };

  struct Packet { // TODO, implement
    int a;
  };
}

inline std::ostream& operator<<(std::ostream& os, dns::DomainName& d) {
  std::list<std::string>::const_iterator it = d.name.begin();
  while (it != d.name.end()) {
    os << *it;
    ++it;
    if (it != d.name.end()) os << '.';
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, dns::Type& t) {
  switch(t) {
  case dns::Type::A: os << "A"; break;
  case dns::Type::NS: os << "NS"; break;
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

inline std::ostream& operator<<(std::ostream& os, dns::ResourceRecord& r) {
  os << r.name <<  " ";
  os << r.QType <<  " ";
  if (r.QType == dns::Type::A) os << r.AValue <<  " ";
  else if (r.QType == dns::Type::NS) os << r.NSValue <<  " ";
  os << r.QClass <<  " ";
  os << r.TTL;
  return os;
}

inline std::istream& operator>>(std::istream& is, dns::Type& t) {
  std::string c;
  is >> c;
  if (c == std::string("A")) t = dns::Type::A;
  else if (c == std::string("NS")) t = dns::Type::NS;
  return is;
}

inline std::istream& operator>>(std::istream& is, dns::DomainName& d) {

  std::string str, sub_domain;
  is >> str;
  std::stringstream ss(str);
  while(getline(ss, sub_domain, '.')) {
    d.name.push_back(sub_domain);
  }
  return is;
}

inline std::istream& operator>>(std::istream& is, dns::ResourceRecord& r) {
  is >> r.name;
  is >> r.QType;
  switch(r.QType) {
  case dns::Type::A: is >> r.AValue; break;
  case dns::Type::NS: is >> r.NSValue; break;
  }
  is >> r.QClass;
  is >> r.TTL;
  return is;
}

#endif