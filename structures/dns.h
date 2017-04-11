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

// Application layer structures
namespace dns {

  enum QR : uint16_t { // Query/Response Flag
    QR_QUERY = 0x8000, 
    QR_ANSWER = 0x0000, 
    QR_MASK = 0x8000
  };

  enum Opcode : uint16_t { // Operation code
    Opcode_QUERY = 0x0000, //0
    Opcode_IQUERY = 0x0800, //1
    Opcode_STATUS = 0x1000, //2
    Opcode_RESERVED = 0x1800, //3
    Opcode_NOTIFY = 0x2000, //4
    Opcode_UPDATE = 0x2800, //5
    Opcode_MASK = 0x7800
  };

  enum AA : uint16_t { // Authoritative Answer Flag
    AA_AUTHORITATIVE_ANSWER = 0x0400,
    AA_NOT_AUTHORITATIVE_ANSWER = 0x0000,
    AA_MASK = 0x0400
  };

  enum TC : uint16_t { // Truncation Flag
    TC_TRUNCATED  = 0x0200,
    TC_NOT_TRUNCATED  = 0x0000,
    TC_MASK  = 0x0200
  };

  enum RD : uint16_t { // Recursion Desired
    RD_RECURSIVE = 0x0100,
    RD_ITERATIVE = 0x0000,
    RD_MASK = 0x0100
  };

  enum RA : uint16_t { // Recursion Available
    RA_RECURSION_AVAILABLE = 0x0080,
    RA_ITERATIVE = 0x0000,
    RA_MASK = 0x0080
  };

  enum Z : uint16_t { // Zero
    Z_RESERVED = 0x0000,
    Z_MASK = 0x0070,
  };

  enum RCode : uint16_t { // Response code
    RCode_NO_ERROR = 0x0000,
    RCode_FORMAT_ERROR = 0x0001,
    RCode_SERVER_FAILURE = 0x0002,
    RCode_NAME_ERROR = 0x0003,
    RCode_NOT_IMPLEMENTED = 0x0004,
    RCode_REFUSED = 0x0005,
    RCode_YX_DOMAIN = 0x0006,
    RCode_YX_RR_SET = 0x0007,
    RCode_NX_RR_SET = 0x0008,
    RCode_NOT_AUTH = 0x0009,
    RCode_NOT_ZONE = 0x000A,
    RCode_MASK = 0x000F
  };

  enum Type : uint16_t {A = 1, NS = 2 };

  enum Class : uint16_t {IN = 1};

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
    Header(const char* const other) {
      std::memcpy(&id,other,2);
      std::memcpy(&flags_code,&other[2],2);
      std::memcpy(&QDCount,&other[4],2);
      std::memcpy(&ANCount,&other[6],2);
      std::memcpy(&NSCount,&other[8],2);
      std::memcpy(&ARCount,&other[10],2);
    }

    void setFlag(uint16_t flag, uint16_t mask) {
      flags_code = (flags_code & ~mask);
      flags_code = (flags_code | flag);
    }

    bool is(uint16_t flag, uint16_t mask) const {
      return (flags_code & mask) == flag;
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

    std::string as_string() const {
      std::string res = "";
      res += "id: " + std::to_string(id) + "\n";
      res += "flags_code: " + std::to_string(flags_code) + "\n";
      res += "QDCount: " + std::to_string(QDCount) + "\n";
      res += "ANCount: " + std::to_string(ANCount) + "\n";
      res += "NSCount: " + std::to_string(NSCount) + "\n";
      res += "ARCount: " + std::to_string(ARCount) + "\n";
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

    std::string as_string() const {
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
    Class QClass;
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
    ResourceRecord(const char* const data) {
      
      // Reading name
      name = DomainName(data);
      int i = name.size();
      
      // Reading type
      std::memcpy(&QType, &data[i], sizeof(QType));
      i += sizeof(QType);

      // Reading value
      if (QType == Type::A) {
        memcpy(&AValue, &data[i], sizeof(AValue));
        i += AValue.size();
      } else if (QType == Type::NS) {
        NSValue = DomainName(&data[i]);
        i += NSValue.size();
      }

      //Reading class
      std::memcpy(&QClass, &data[i], sizeof(QClass));
      i += sizeof(QClass);

      // Reading TTL
      std::memcpy(&TTL, &data[i], sizeof(TTL));
    }

    const char* c_str() const {
      char* res = new char[this->size()];
      for (int i = 0; i < this->size(); ++i) res[i] = 0x00;
      
      // copying name
      int i = name.size();
      const char* name_str = name.c_str();
      std::memcpy(res,name_str,i);
      delete[] name_str;

      // copying type
      std::memcpy(&res[i],&QType,sizeof(QType));
      i += sizeof(QType);
      
      // copying value
      if (QType == Type::A) {
        memcpy(&res[i],AValue.ip,AValue.size());
        i += AValue.size(); 
      } else if (QType == Type::NS) {
        const char * nsv_str = NSValue.c_str();
        memcpy(&res[i],nsv_str,NSValue.size());
        delete[] nsv_str;
        i += NSValue.size();
      }

      // copying Class
      std::memcpy(&res[i],&QClass,sizeof(QClass));
      i += sizeof(QClass);

      // copying TTL
      std::memcpy(&res[i],&TTL,sizeof(TTL));
      i += sizeof(TTL);

      return res;
    }

    int size() const {
      int size = name.size() + sizeof(QType) + sizeof(QClass) + sizeof(TTL);
      if (QType == Type::A) size += AValue.size();
      else if (QType == Type::NS) size += NSValue.size();
      return size;
    }

    std::string as_string() const {
      std::string res = "";
      res += this->name.as_string() + " ";
      switch(this->QType) {
      case dns::Type::A: res += "A "; res += this->AValue.as_string() + " "; break;
      case dns::Type::NS: res += "NS "; res += this->NSValue.as_string() + " "; break;
      }
      switch(this->QType) {
      case dns::Class::IN: res += "IN "; res += this->AValue.as_string() + " "; break;
      }
      res += std::to_string(this->TTL) + " ";
      return res;
    }
  };

  struct Packet {
    Header header;
    std::list<ResourceRecord> questions;
    std::list<ResourceRecord> answers;
    std::list<ResourceRecord> authoritatives;
    std::list<ResourceRecord> aditionals;

    Packet() {
      header.QDCount = 0;
      header.ANCount = 0;
      header.NSCount = 0;
      header.ARCount = 0;
    }
    Packet(const Packet& other) {
      header = other.header;
      questions = other.questions;
      answers = other.answers;
      authoritatives = other.authoritatives;
      aditionals = other.aditionals;
    }
    Packet(const char* const data) {
      
      int i = 0;
      header = Header(data);
      i += header.size();
      
      ushort j;
      for (j=0; j<header.QDCount; ++j) {
        questions.push_back(ResourceRecord(&data[i]));
        i += questions.back().size();
      }
      for (j=0; j<header.ANCount; ++j) {
        answers.push_back(ResourceRecord(&data[i]));
        i += answers.back().size();
      }
      for (j=0; j<header.NSCount; ++j) {
        authoritatives.push_back(ResourceRecord(&data[i]));
        i += authoritatives.back().size();
      }
      for (j=0; j<header.ARCount; ++j) {
        aditionals.push_back(ResourceRecord(&data[i]));
        i += aditionals.back().size();
      }
    }

    void addQuestionResource(ResourceRecord r) {
      questions.push_back(r);
      header.QDCount++;
    }

    void addAnswerResource(ResourceRecord r) {
      questions.push_back(r);
      header.QDCount++;
    }

    void addAuthoritativeResource(ResourceRecord r) {
      questions.push_back(r);
      header.QDCount++;
    }

    void addAditionalResource(ResourceRecord r) {
      questions.push_back(r);
      header.QDCount++;
    }

    const char* c_str() const {
      std::list<ResourceRecord>::const_iterator it;

      int len = this->size();
      char* res = new char[len];
      for (int j = 0; j < len; ++j) res[j] = 0x00;

      const char* h_chars = header.c_str();
      std::memcpy(res, h_chars, header.size());
      delete[] h_chars;

      int i = header.size();
      for (it = questions.begin(); it != questions.end(); ++it) {
        const char* r_chars = it->c_str();
        std::memcpy(&res[i],r_chars,it->size());
        delete[] r_chars;
        i += it->size();
      }
      for (it = answers.begin(); it != answers.end(); ++it) {
        const char* r_chars = it->c_str();
        std::memcpy(&res[i],r_chars,it->size());
        delete[] r_chars;
        i += it->size();
      }
      for (it = authoritatives.begin(); it != authoritatives.end(); ++it) {
        const char* r_chars = it->c_str();
        std::memcpy(&res[i],r_chars,it->size());
        delete[] r_chars;
        i += it->size();
      }
      for (it = aditionals.begin(); it != aditionals.end(); ++it) {
        const char* r_chars = it->c_str();
        std::memcpy(&res[i],r_chars,it->size());
        delete[] r_chars;
        i += it->size();
      }

      return res;
    }

    int size() const {
      int len = header.size();
      std::list<ResourceRecord>::const_iterator it;
      
      for (it = questions.begin(); it != questions.end(); ++it) {
        len += it->size();
      }
      for (it = answers.begin(); it != answers.end(); ++it) {
        len += it->size();
      }
      for (it = authoritatives.begin(); it != authoritatives.end(); ++it) {
        len += it->size();
      }
      for (it = aditionals.begin(); it != aditionals.end(); ++it) {
        len += it->size();
      }
      return len;
    }

    std::string as_string() const {

      std::string res = "-------------- dns packet ---------------\n";

      res += "** HEADER ** \n";
      res += header.as_string() + "\n\n";
  
      res += "\n\n** QUESTION SECTION ** \n";
      std::list<dns::ResourceRecord>::const_iterator it;
      for (it = questions.begin(); it != questions.end(); ++it) {
        res += it->as_string() + "\n";
      }
      
      res += "\n\n** ANSWER SECTION ** \n";
      for (it = answers.begin(); it != answers.end(); ++it) {
        res += it->as_string() + "\n";
      }
      
      res += "\n\n** AUTHORITATIVES SECTION ** \n";
      for (it = authoritatives.begin(); it != authoritatives.end(); ++it) {
        res += it->as_string() + "\n";
      }
      
      res += "\n\n** ADITIONAL SECTION ** \n";
      for (it = aditionals.begin(); it != aditionals.end(); ++it) {
        res += it->as_string() + "\n";
      }

      res += "-----------------------------------------\n";
      
      return res;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const dns::DomainName& d) {
  std::list<std::string>::const_iterator it = d.name.begin();
  while (it != d.name.end()) {
    os << *it;
    ++it;
    if (it != d.name.end()) os << '.';
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const dns::Type& t) {
  switch(t) {
  case dns::Type::A: os << "A"; break;
  case dns::Type::NS: os << "NS"; break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const dns::QR& qr) {
  switch(qr) {
  case dns::QR::QR_QUERY: os << "QR_QUERY"; break;
  case dns::QR::QR_ANSWER: os << "QR_ANSWER"; break;
  default: os << "INV"; break;
  }
  return os;  
}

inline std::ostream& operator<<(std::ostream& os, const dns::Opcode& o) {
  switch(o) {
  case dns::Opcode::Opcode_QUERY: os << "Opcode_QUERY"; break;
  case dns::Opcode::Opcode_IQUERY: os << "Opcode_IQUERY"; break;
  case dns::Opcode::Opcode_STATUS: os << "Opcode_STATUS"; break;
  case dns::Opcode::Opcode_RESERVED: os << "Opcode_RESERVED"; break;
  case dns::Opcode::Opcode_NOTIFY: os << "Opcode_NOTIFY"; break;
  case dns::Opcode::Opcode_UPDATE: os << "Opcode_UPDATE"; break;
  default: os << "INV"; break;
  }
  return os;  
}

inline std::ostream& operator<<(std::ostream& os, const dns::AA& r) {
  switch(r) {
  case dns::AA::AA_AUTHORITATIVE_ANSWER: os << "AA_AUTHORITATIVE_ANSWER"; break;
  case dns::AA::AA_NOT_AUTHORITATIVE_ANSWER: os << "AA_NOT_AUTHORITATIVE_ANSWER"; break;
  default: os << "INV"; break;
  }
  return os;  
}

inline std::ostream& operator<<(std::ostream& os, const dns::TC& tc) {
  switch(tc) {
  case dns::TC::TC_TRUNCATED: os << "TC_TRUNCATED"; break;
  case dns::TC::TC_NOT_TRUNCATED: os << "TC_NOT_TRUNCATED"; break;
  default: os << "INV"; break;
  }
  return os;  
}

inline std::ostream& operator<<(std::ostream& os, const dns::RD& r) {
  switch(r) {
  case dns::RD::RD_RECURSIVE: os << "RD_RECURSIVE"; break;
  case dns::RD::RD_ITERATIVE: os << "RD_ITERATIVE"; break;
  default: os << "INV"; break;
  }
  return os;  
}

inline std::ostream& operator<<(std::ostream& os, const dns::RA& r) {
  switch(r) {
  case dns::RA::RA_RECURSION_AVAILABLE: os << "RA_RECURSION_AVAILABLE"; break;
  case dns::RA::RA_ITERATIVE: os << "RA_ITERATIVE"; break;
  default: os << "INV"; break;
  }
  return os;  
}

inline std::ostream& operator<<(std::ostream& os, const dns::RCode& r) {
  switch(r) {
  case dns::RCode::RCode_NO_ERROR: os << "RCode_NO_ERROR"; break;
  case dns::RCode::RCode_FORMAT_ERROR: os << "RCode_FORMAT_ERROR"; break;
  case dns::RCode::RCode_SERVER_FAILURE: os << "RCode_SERVER_FAILURE"; break;
  case dns::RCode::RCode_NAME_ERROR: os << "RCode_NAME_ERROR"; break;
  case dns::RCode::RCode_NOT_IMPLEMENTED: os << "RCode_NOT_IMPLEMENTED"; break;
  case dns::RCode::RCode_REFUSED: os << "RCode_REFUSED"; break;
  case dns::RCode::RCode_YX_DOMAIN: os << "RCode_YX_DOMAIN"; break;
  case dns::RCode::RCode_YX_RR_SET: os << "RCode_YX_RR_SET"; break;
  case dns::RCode::RCode_NX_RR_SET: os << "RCode_NX_RR_SET"; break;
  case dns::RCode::RCode_NOT_AUTH: os << "RCode_NOT_AUTH"; break;
  case dns::RCode::RCode_NOT_ZONE: os << "RCode_NOT_ZONE"; break;
  default: os << "INV"; break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const dns::Class& t) {
  switch(t) {
  case dns::Class::IN: os << "IN"; break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const dns::Header& h) {
  os << "id: " << h.id << std::endl;

  os << "flags: ";
  os << (dns::QR)(h.flags_code & dns::QR::QR_MASK) << "|";
  os << (dns::Opcode)(h.flags_code & dns::Opcode::Opcode_MASK) << "|";
  os << (dns::AA)(h.flags_code & dns::AA::AA_MASK) << "|";
  os << (dns::TC)(h.flags_code & dns::TC::TC_MASK) << "|";
  os << (dns::RD)(h.flags_code & dns::RD::RD_MASK) << "|";
  os << (dns::RA)(h.flags_code & dns::RA::RA_MASK) << "|";
  os << (dns::RCode)(h.flags_code & dns::RCode::RCode_MASK);

  os << std::endl;
  os << "QDCount: " << h.QDCount << std::endl;
  os << "ANCount: " << h.ANCount << std::endl;
  os << "NSCount: " << h.NSCount << std::endl;
  os << "ARCount: " << h.ARCount;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const dns::ResourceRecord& r) {
  os << r.name <<  " ";
  os << r.QType <<  " ";
  if (r.QType == dns::Type::A) os << r.AValue <<  " ";
  else if (r.QType == dns::Type::NS) os << r.NSValue <<  " ";
  os << r.QClass <<  " ";
  os << r.TTL;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const dns::Packet& p) {
  os << "-------------- dns packet ---------------" << std::endl;
  os << "** HEADER ** " << std::endl << p.header << std::endl;
  
  os << std::endl;
  os << "** QUESTION SECTION **" << std::endl;
  std::list<dns::ResourceRecord>::const_iterator it;
  for (it = p.questions.begin(); it != p.questions.end(); ++it) {
    os << *it << std::endl;
  }
  
  os << std::endl;
  os << "** ANSWER SECTION **" << std::endl;
  for (it = p.answers.begin(); it != p.answers.end(); ++it) {
    os << *it << std::endl;
  }
  
  os << std::endl;
  os << "** AUTHORITATIVES SECTION **" << std::endl;
  for (it = p.authoritatives.begin(); it != p.authoritatives.end(); ++it) {
    os << *it << std::endl;
  }
  
  os << std::endl;
  os << "** ADITIONAL SECTION **" << std::endl;
  for (it = p.aditionals.begin(); it != p.aditionals.end(); ++it) {
    os << *it << std::endl;
  }

  os << "-----------------------------------------" << std::endl;
  return os;
}

inline std::istream& operator>>(std::istream& is, dns::Type& t) {
  std::string c;
  is >> c;
  if (c == std::string("A")) t = dns::Type::A;
  else if (c == std::string("NS")) t = dns::Type::NS;
  return is;
}

inline std::istream& operator>>(std::istream& is, dns::Class& t) {
  std::string c;
  is >> c;
  if (c == std::string("IN")) t = dns::Class::IN;
  return is;
}

inline std::istream& operator>>(std::istream& is, dns::DomainName& d) {

  d.name.clear();
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

inline std::istream& operator>>(std::istream& is, dns::Header& h) {
  is >> h.id;
  is >> h.flags_code;
  is >> h.QDCount;
  is >> h.ANCount;
  is >> h.NSCount;
  is >> h.ARCount;
  return is;
}

inline std::istream& operator>>(std::istream& is, dns::Packet& p) {
  
  is >> p.header;
  dns::ResourceRecord r;
  ushort j;

  for (j=0; j<p.header.QDCount; ++j) {
    is >> r;
    p.questions.push_back(r);
  }
  for (j=0; j<p.header.ANCount; ++j) {
    is >> r;
    p.answers.push_back(r);
  }
  for (j=0; j<p.header.NSCount; ++j) {
    is >> r;
    p.authoritatives.push_back(r);
  }
  for (j=0; j<p.header.ARCount; ++j) {
    is >> r;
    p.aditionals.push_back(r);
  }
  return is;
}

#endif