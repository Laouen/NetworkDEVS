#if !defined dns_h
#define dns_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort
#include <cstring> // memcpy
#include <list>
#include <vector>
#include <sstream> // istringstring
#include <iostream>

#include <string>
#include <fstream>

#include "abstract_types.h"
#include "ipv4.h"


/**
 * @namespace dns
 * 
 * namespace to use for all the data type refered to the DNS protocol.
 */
namespace dns {

  /**
   * @enum QR 
   * 
   * @brief valid values for the Query/Response Flag for dns::Header
   */
  enum QR : uint16_t {
    QR_QUERY = 0x8000, 
    QR_ANSWER = 0x0000, 
    QR_MASK = 0x8000
  };

  /**
   * @enum Opcode 
   * 
   * @brief valid values for the Opcode Flag for dns::Header
   */
  enum Opcode : uint16_t { // Operation code
    Opcode_QUERY = 0x0000, //0
    Opcode_IQUERY = 0x0800, //1
    Opcode_STATUS = 0x1000, //2
    Opcode_RESERVED = 0x1800, //3
    Opcode_NOTIFY = 0x2000, //4
    Opcode_UPDATE = 0x2800, //5
    Opcode_MASK = 0x7800
  };

  /**
   * @enum AA 
   * 
   * @brief valid values for the AA Flag for dns::Header
   */
  enum AA : uint16_t { // Authoritative Answer Flag
    AA_AUTHORITATIVE_ANSWER = 0x0400,
    AA_NOT_AUTHORITATIVE_ANSWER = 0x0000,
    AA_MASK = 0x0400
  };

  /**
   * @enum TC 
   * 
   * @brief valid values for the TC Flag for dns::Header
   */
  enum TC : uint16_t { // Truncation Flag
    TC_TRUNCATED  = 0x0200,
    TC_NOT_TRUNCATED  = 0x0000,
    TC_MASK  = 0x0200
  };

  /**
   * @enum RD 
   * 
   * @brief valid values for the RD Flag for dns::Header
   */
  enum RD : uint16_t { // Recursion Desired
    RD_RECURSIVE = 0x0100,
    RD_ITERATIVE = 0x0000,
    RD_MASK = 0x0100
  };

  /**
   * @enum RA 
   * 
   * @brief valid values for the RA Flag for dns::Header
   */
  enum RA : uint16_t { // Recursion Available
    RA_RECURSION_AVAILABLE = 0x0080,
    RA_ITERATIVE = 0x0000,
    RA_MASK = 0x0080
  };

  /**
   * @enum Z 
   * 
   * @brief valid values for the Z Flag for dns::Header
   */
  enum Z : uint16_t { // Zero
    Z_RESERVED = 0x0000,
    Z_MASK = 0x0070,
  };

  /**
   * @enum RCode 
   * 
   * @brief valid values for the RCode Flag for dns::Header
   */
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

  /**
   * @enum Type 
   * 
   * @brief valid values for the Type Flag for dns::Header
   */
  enum Type : uint16_t {A = 1, NS = 2 };

  /**
   * @enum Class 
   * 
   * @brief valid values for the Class Flag for dns::Header
   */
  enum Class : uint16_t {IN = 1};

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct dns::Header dns.h
   * 
   * @brief Structure that implements a dns::Packet header.
   * 
   * @details For a detailed documentation of the meaning of each field can
   * be found in http://www.tcpipguide.com/free/t_DNSMessageHeaderandQuestionSectionFormat.htm
   * 
   */
  struct Header : abstract::Header {
    ushort id;
    ushort flags_code;
    ushort QDCount;
    ushort ANCount;
    ushort NSCount;
    ushort ARCount;

    /**
     * @brief Default constructor.
     * @details Inicializes a new instance.
     */
    Header() {}

    /**
     * @brief Copy contructor.
     * 
     * @param other A dns::Header to copy its value to create the new instance. 
     */
    Header(const Header& other) {
      id = other.id;
      flags_code = other.flags_code;
      QDCount = other.QDCount;
      ANCount = other.ANCount;
      NSCount = other.NSCount;
      ARCount = other.ARCount;
    }

    /**
     * @brief Contructor from const char*
     * @details This constructor takes a const char * pointing to a memory block
     * that contains a representation of a dns::Header as the one obtained by 
     * the c_str method of this class.
     * 
     * @param other A const char* pointing to the correctly formated memory block
     */    
    Header(const char* const other) {
      std::memcpy(&id,other,2);
      std::memcpy(&flags_code,&other[2],2);
      std::memcpy(&QDCount,&other[4],2);
      std::memcpy(&ANCount,&other[6],2);
      std::memcpy(&NSCount,&other[8],2);
      std::memcpy(&ARCount,&other[10],2);
    }

    /**
     * @brief Uses the mask to set the correct bits of the flags_code field of 
     * the instance with the value the bits in the flag parameter in those positions.
     * 
     * @details The bits with 1s in the mask are the bits of the flags_code that 
     * will be modified by this method.
     * 
     * @param flag The values to set in the flags_code.
     * @param mask The positions of the flags_code to modify.
     */
    void setFlag(uint16_t flag, uint16_t mask) {
      flags_code = (flags_code & ~mask);
      flags_code = (flags_code | flag);
    }

    /**
     * @brief Checks whether the flags_code is set with the flag passed as parameter.
     * 
     * @param flag The flag to check if it is set or not.
     * @param mask The mask to know witch bits if the flags_code are the bits of the 
     * flag.
     * 
     * @return [description]
     */
    bool is(uint16_t flag, uint16_t mask) const {
      return (flags_code & mask) == flag;
    }

    /**
     * @return The size of the instance in bytes.
     */
    ushort size() const {
      return 12;
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @details This method can be used to store a dns::Header as an array of chars 
     * and be reconstructed with the const char * constructor later.
     * 
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
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

    /**
     * @brief Returns a string formated with the dns::Header value of the this.
     * @details This method can be used to print the dns::HEader value
     * 
     * @return An std::string with the formated value.
     */
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

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct dns::DomainName dns.h
   * 
   * @brief This struct handles domain names with the format 'zone1.zone2.zone3...zoneN'
   * 
   * @details This method allows to parse domain names from strings. An example of
   * a dns::DomainName is 'networks.devs.com' where 'networks' es zone1 
   * 'devs' is zone2 and 'com' is zone3.
   */
  struct DomainName {
    std::vector<std::string> name;

    /**
     * @brief Default constructor
     * @details Construct a new instance with an empty vale ''
     */
    DomainName() {};

    /**
     * @brief Copy constructor
     * 
     * @param other A dns::DomainName to copy its value to construct the new instance.
     */
    DomainName(const DomainName& other) {
      name = other.name;
    }

    /**
     * @brief Constructor from std::string
     * @details This constructor uses a std::string formated in the IDN format.
     * 
     * @param str A std::string used to construct the new instance.
     */
    DomainName(std::string str) {
      std::stringstream ss(str);
      std::string sub_domain;

      while(getline(ss, sub_domain, '.')) {
        name.push_back(sub_domain);
      }
    }

    /**
     * @brief Constructor from const char *
     * @details This constructor uses a char * representation to initialize 
     * the new instance. the format must be the next:
     * 
     * 0xlen1zone10xlen2zone20xlen3...0xlenNzoneN0x00
     * where there is no space between the len and the zones name. The len are 
     * unsigned chars with the length in bytes of the zones names in text format 
     * and the len = 0x00 represent the end of the domain name.
     * 
     * example 0x08networks0x04devs0x03com0x00 = networks.devs.com 
     * 
     * @param data A const char * used to inicialize the new instance
     */
    DomainName(const char* data) {
      int i = 0;
      while (data[i] != 0x00) { /* data[i] == 0 */
        int len = data[i];
        name.push_back(std::string(&data[i+1], len));
        i += len + 1;
      }
    }


    /**
     * @return A const char * pointing to a memory block of a well defined 
     * representation as required by the constructor from const char * of 
     * the instance domain name.
     */
    const char* c_str() const {
      char* res = new char[this->size()];
      for (int i = 0; i < this->size(); ++i) res[i] = 0x00; // last 0x0 character is initialized here
      int len;
      int i = 0;
      std::vector<std::string>::const_iterator it;
      for (it = name.begin(); it != name.end(); ++it) {
        len = it->length();
        res[i] = (unsigned char)len;
        std::memcpy(&res[i+1],it->c_str(),len);
        i += len + 1;
      }
      return res;
    }

    /**
     * @brief Return the value of the instance in a std::string with the IDN format
     * as required by the constructor from std::string.
     * 
     * @return A std::string with the formated domain name value.
     */
    std::string as_string() const {
      std::string res = "";
      std::vector<std::string>::const_iterator it = name.begin();
      while (it != name.end()) {
        res += *it;
        ++it;
        if (it != name.end()) res += '.';
      }
      return res;
    }

    /**
     * @return A int with the zise of the current domain name with is the sum
     * of the size in bytes of all the zones of the domain.
     */
    int size() const {
      int size = 0;
      std::vector<std::string>::const_iterator it;
      for (it = name.begin(); it != name.end(); ++it) {
        size += it->length() + 1;
      }
      return size+1; // last 0 char value
    }

    /**
     * @brief Operator == for dns::DomainName
     * 
     * @return True if both the implicit this and the other instances are equals, 
     * False otherwise.
     */
    bool operator==(const DomainName& other) const {
      if (name.size() != other.name.size()) {
        return false;
      }

      for(ushort i=0; i<name.size(); ++i) {
        if (name[i] != other.name[i]) return false;
      }

      return true;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct dns::ResourceRecord
   * 
   * @brief This structs represent a ResourceRecord that contains information
   * of a query or answer of a dns::domainName.
   * 
   * @details This struct stores the next information:
   * * name: dns::DomainName that is query.
   * * Qtype: The type of answer that was returned by a server.
   * * AValue: If the answer is of type A, the IPv4 value of the response.
   * * NSValue: Of the answer is of type NS, the dns::DomainName value of a server to send the query.
   * * QClass: The class of query.
   * * TTL: The Time To Live of the response in cache.
   */
  struct ResourceRecord {
    DomainName name;
    Type QType;
    IPv4 AValue;
    DomainName NSValue;
    Class QClass;
    ushort TTL;

    /**
     * @brief Default Constructor.
     * @details Construct an empty new instance of dns::ResourceRecord.
     */
    ResourceRecord() {}
    
    /**
     * @brief Copy constructor
     * 
     * @param other A dns::ResourceRecod to copy in the new instance. 
     */
    ResourceRecord(const ResourceRecord& other) {
      name = other.name;
      AValue = other.AValue;
      NSValue = other.NSValue;
      QType = other.QType;
      QClass = other.QClass;
      TTL = other.TTL;
    }

    /**
     * @brief Contructor from const char*
     * @details This constructor takes a const char * pointing to a memory block
     * that contains a representation of a dns::RecordResource as the one obtained by 
     * the c_str method of this class.
     * 
     * @param data A const char* pointing to the correctly formated memory block
     */    
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

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @details This method can be used to store a dns::ResourceRecord as an 
     * array of chars and be reconstructed with the const char * constructor later.
     * 
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
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

    /**
     * @return An int with the instance size in bytes.
     */
    int size() const {
      int size = name.size() + sizeof(QType) + sizeof(QClass) + sizeof(TTL);
      if (QType == Type::A) size += AValue.size();
      else if (QType == Type::NS) size += NSValue.size();
      return size;
    }

    /**
     * @brief It print the value of the instance in a human redable version on 
     * a std::string.
     * 
     * @return A std::string with the instance value printed on it.
     */
    std::string as_string() const {
      std::string res = "";
      res += this->name.as_string() + " ";
      switch(this->QType) {
      case dns::Type::A: res += "A "; res += this->AValue.as_string() + " "; break;
      case dns::Type::NS: res += "NS "; res += this->NSValue.as_string() + " "; break;
      }
      switch(this->QClass) {
      case dns::Class::IN: res += "IN "; break;
      }
      res += std::to_string(this->TTL) + " ";
      return res;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct dns::Packet dns.h
   * 
   * @brief This struct represent a dns::Packet, it has a dns::Header that
   * indicates how many dns::resourceRecord there is in each one of the four 
   * sections: questions, answers, authoritatives and aditionals and four 
   * std::lists<dns::ResourceRedords> containing them.
   * 
   * @details This struct carry a DNS query and answer though the network.
   */
  struct Packet {
    Header header;
    std::list<ResourceRecord> questions;
    std::list<ResourceRecord> answers;
    std::list<ResourceRecord> authoritatives;
    std::list<ResourceRecord> aditionals;

    /**
     * @brief Default constructor
     * @details It initilizes a new empty instance with no dns::ResourceRedords
     */
    Packet() {
      header.QDCount = 0;
      header.ANCount = 0;
      header.NSCount = 0;
      header.ARCount = 0;
    }

    /**
     * @brief Copy constructor
     * @details It construct a new instance copying the value from the instances
     * passed as parameter.
     * 
     * @param other A dns::Packet used to initilize the new instance.
     */
    Packet(const Packet& other) {
      header = other.header;
      questions = other.questions;
      answers = other.answers;
      authoritatives = other.authoritatives;
      aditionals = other.aditionals;
    }

    /**
     * @brief Contructor from const char*
     * @details This constructor takes a const char * pointing to a memory block
     * that contains a representation of a dns::Packet as the one obtained by 
     * the c_str method of this class.
     * 
     * @param data A const char* pointing to the correctly formated memory block
     */ 
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

    /**
     * @brief Adds a new dns::ResourceRedord to the questions section
     * 
     * @param r A dns::ResourceRecord to add.
     */
    void addQuestionResource(ResourceRecord r) {
      questions.push_back(r);
      header.QDCount++;
    }

    /**
     * @brief Adds a new dns::ResourceRedord to the answers section
     * 
     * @param r A dns::ResourceRecord to add.
     */
    void addAnswerResource(ResourceRecord r) {
      answers.push_back(r);
      header.ANCount++;
    }

    /**
     * @brief Adds a new dns::ResourceRedord to the authoritatives section
     * 
     * @param r A dns::ResourceRecord to add.
     */
    void addAuthoritativeResource(ResourceRecord r) {
      authoritatives.push_back(r);
      header.NSCount++;
    }

    /**
     * @brief Adds a new dns::ResourceRedord to the aditional section
     * 
     * @param r A dns::ResourceRecord to add.
     */
    void addAditionalResource(ResourceRecord r) {
      aditionals.push_back(r);
      header.ARCount++;
    }

    /**
     * @brief Copy all the bytes of the instance in a memory block and return a
     * pointer to that memory block.
     * 
     * @details This method can be used to store a dns::Packet as an 
     * array of chars and be reconstructed with the const char * constructor later.
     * 
     * @return A const char * that point to the memory block where the instance 
     * was capied.
     */
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

    /**
     * @return A int with the size of the instance in bytes.
     */
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

    /**
     * @brief prints the dns::Packet instance in a std::string in a human 
     * redable format.
     *
     * @return a std::string with the printed value of the instance.
     */
    std::string as_string() const {

      std::string res = "-------------- dns packet ---------------\n";

      res += "** HEADER ** \n";
      res += header.as_string();
  
      res += "\n** QUESTION SECTION ** \n";
      std::list<dns::ResourceRecord>::const_iterator it;
      for (it = questions.begin(); it != questions.end(); ++it) {
        res += it->as_string() + "\n";
      }
      
      res += "\n** ANSWER SECTION ** \n";
      for (it = answers.begin(); it != answers.end(); ++it) {
        res += it->as_string() + "\n";
      }
      
      res += "\n** AUTHORITATIVES SECTION ** \n";
      for (it = authoritatives.begin(); it != authoritatives.end(); ++it) {
        res += it->as_string() + "\n";
      }
      
      res += "\n** ADITIONAL SECTION ** \n";
      for (it = aditionals.begin(); it != aditionals.end(); ++it) {
        res += it->as_string() + "\n";
      }

      res += "-----------------------------------------";
      
      return res;
    }
  };

  /**
   * 
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct dns::Zone dns.h
   * 
   * @brief This struct represent a zone entry that has the zone domain name 
   * and the domain name of the server that is authoritative for that zone.
   */
  struct Zone {
    dns::ResourceRecord name_server; 
    dns::ResourceRecord authoritative;

    /**
     * @return True if the zone is empty, False otherwise
     */
    bool empty() const {
      return  name_server.name.name.size() == 0 ||
              authoritative.name.name.size() == 0 ||
              name_server.QType != Type::NS ||
              authoritative.QType != Type::A;
    }

    /**
     * @brief prints the dns::Zone instance in a std::string in a human 
     * redable format.
     *
     * @return a std::string with the printed value of the instance.
     */
    std::string as_string() const {
      std::string res = "\n";
      res += "-------------- Zone --------------\n";
      res += "** NS **\n";
      res += name_server.as_string() + "\n";
      res += "** A **\n";
      res += authoritative.as_string() + "\n";
      res += "----------------------------------";
      return res;
    }

    /**
     * @brief Checks whether the instance zone is authoritative for a 
     * given dns::DomainName or not. 
     * 
     * @param d A dns::DomainName to check if is under the zone responsability or not.
     * @return True if the zone is responzible of the dns::DomainName, False otherwise.
     */
    bool isZoneOf(dns::DomainName d) const {
      int name_server_len = name_server.name.name.size();
      int domain_len = d.name.size();

      if (name_server_len > domain_len) {
        return false;
      }

      for(int i = 1; i <= name_server_len; ++i) {
        if (name_server.name.name[name_server_len-i] != d.name[domain_len-i]) {
          return false;
        }
      }

      return true;
    }
  };
}

inline std::ostream& operator<<(std::ostream& os, const dns::DomainName& d) {
  std::vector<std::string>::const_iterator it = d.name.begin();
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

  os << "flags: " << std::endl;
  os << "\t" << (dns::QR)(h.flags_code & dns::QR::QR_MASK) << std::endl;
  os << "\t" << (dns::Opcode)(h.flags_code & dns::Opcode::Opcode_MASK) << std::endl;
  os << "\t" << (dns::AA)(h.flags_code & dns::AA::AA_MASK) << std::endl;
  os << "\t" << (dns::TC)(h.flags_code & dns::TC::TC_MASK) << std::endl;
  os << "\t" << (dns::RD)(h.flags_code & dns::RD::RD_MASK) << std::endl;
  os << "\t" << (dns::RA)(h.flags_code & dns::RA::RA_MASK) << std::endl;
  os << "\t" << (dns::RCode)(h.flags_code & dns::RCode::RCode_MASK);

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

inline std::istream& operator>>(std::istream& is, dns::Zone& z) {
  is >> z.name_server;
  is >> z.authoritative;
  return is;
}

#endif