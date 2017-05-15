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
 * @brief This struct declares a IPv4 type for NetworkDEVS simulation purposes. 
 *
 * @details It's not meant to be a full IPv4 struct, for that purpose, there is 
 * many external C++ libraries. This struct represent an IPv4 as a unsigned short 
 * array of length 4 and it was designed to easely simulate Networks using the 
 * NetworkDEVS framework.
 *
 */
struct IPv4 {
	ushort ip[4];

  /**
   * @brief Default constructor
   * @details Initialices a new IPv4 instance with 0.0.0.0
   */
  IPv4() {
    for(int i=0; i<4; ++i)
      ip[i] = 0;
  }

  /**
   * @brief Constructor from std::String
   * @details It construct a new IPv4 instance using the std::string >> operator 
   * to read values with string format 'a.b.c.d' where 0 <= a,b,c,d <= 255 in 
   * decimal representation.
   * 
   * @param other_ip A string representing the IPv4 to construct.
   */
  IPv4(const std::string& other_ip) {
    char ch;
    std::istringstream strm(other_ip);
    strm >> ip[0] >> ch >> ip[1] >> ch >> ip[2] >> ch >> ip[3];
  }

  /**
   * @brief Constructor from const char *
   * @details This constructor uses a const char * as the representation where 
   * to interpreat the IPv4 value to create.
   * 
   * @param other_ip A const char * that points to a representation of the IPv4 
   * in the next form: 'a.b.c.d' where 0 <= a,b,c,d <= 255 in decimal representation. 
   * This constructor fails if the pointer is not pointing to four consecutives 
   * unsigned short values and the new instance is created with trash values.
   */
  IPv4(const char* other_ip) {
    char ch;
    std::istringstream strm(other_ip);
    strm >> ip[0] >> ch >> ip[1] >> ch >> ip[2] >> ch >> ip[3];
  }

  /**
   * @brief Constructor from unsigned short *
   * @details This constructor takes an unsigned short * and reads from it four
   * consecutives usigned short a,b,c and d to construct a new IPv4 with 
   * value a.b.c.d.
   * 
   * @param other_ip An unsigned pointer short that must point to four consecutives 
   * unsigned short values used to construct the new IPv4 instance. 
   * This constructor fails if the pointer is not pointing to four consecutives 
   * unsigned short values and the new instance is created with trash values.
   */
  IPv4(ushort* other_ip) {
    for(int i=0; i<4; ++i)
      ip[i] = other_ip[i];
  }

  /**
   * @brief Copy constructor
   * @details This constructor uses an IPv4 instance to construct a copy of its
   * value. This constructor is the one used by C++ in the IPv4 default == operator.
   * 
   * @param other_ip An IPv4 instance to be copied.
   */
  IPv4(const IPv4& other_ip) {
    for(int i=0; i<4; ++i)
      ip[i] = other_ip.ip[i];
  }

  /**
   * @brief Equal comparator operator ==
   * @details This operator checks if the implicit IPv4 instance and the other
   * instance passed as parameter are equals or not. Two IPv4 instances 
   * are equals when their four unsigned short values are equals.
   * 
   * @param other The IPv4 instance to compare the implicit this IPv4 instance.
   * @return True if both other and the this instance are equals, False otherwise.
   */
  bool operator==(const IPv4& other) const {
    bool res = true;
    for(int i=0; i<4; ++i)
      res = res && ip[i] == other.ip[i];
    return res;
  }

  /**
   * @brief Lower than comparator
   * @details This operator compares the implicit this instance agains the other
   * instances passed as parameter to return whether the this is lower than 
   * the other or not. An IPv4 A is lower than another IPv4 B when the first 
   * different value from left to right is lower in the A instance than in the B
   * instance. 
   * 
   * @param other The instance against what to compare the implicite instance.  
   * @return True if this is lower than other, False otherwise.
   */
  bool operator<(const IPv4& other) const {
    for(int i=0; i<4; ++i) {
      if (ip[i] < other.ip[i])
        return true;
      else if (ip[i] > other.ip[i])
        return false;
    }
    return false;
  }

  /**
   * @brief Returns a string formated as 'a.b.c.d' with the IPv4 value of the this.
   * @details This method can be used to print the IPv4 value, the format used
   * to print the value in a string is consistent with the format expected by
   * the std::string constructor.
   * @return An std::string with the formated value.
   */
  std::string as_string() const {
    std::string res = "";
    for (int i = 0; i < 4; ++i) {
      res += std::to_string(ip[i]);
      if(i<3) res += ".";
    }
    return res;
  }

  /**
   * @brief Bitwise & operator
   * @details This operator implement a bitwise & operation between the value of
   * the implicit this instance and the other instance passed as parameter and 
   * returns the value of the operation. The this instances is not modified.
   * Example: 255.0.255.1 & 0.245.255.2 = 0.0.255.0
   * 
   * @param other An IPv4 instance.
   * @return An IPv4 instance with the result of this & other.
   */
  IPv4 operator&(const IPv4& other) const {
    IPv4 res;
    for(int i=0; i<4; ++i)
      res.ip[i] = ip[i] & other.ip[i];
    return res;
  }

  /**
   * @brief Bitwise | operator
   * @details This operator implement a bistwise | operation between the value of
   * the implicit this instance and the other instance passed as parameter nad 
   * returns the value of the operation. The this instance is not modified.
   * Example 255.0.255.1 | 0.245.255.2 = 255.245.255.3
   * 
   * @param other An IPv4 instance 
   * @return An IPv4 instance with the result of this | other.
   */
  IPv4 operator|(const IPv4& other) const {
    IPv4 res;
    for(int i=0; i<4; ++i)
      res.ip[i] = ip[i] | other.ip[i];
    return res;
  }

  /**
   * @brief Return the size of an IPv4 instance.
   * @details The size is 4 * sizeof(unsigned short), this size can be different
   * depending on the used compiler.
   * @return An integer with the compiler size of the IPv4 type.
   */
  int size() const {
    return 4 * sizeof(ip[0]);
  }

  /**
   * @brief << operator to print the IPv4 value in a std::ostream
   * @details The format used to print the IPv4 value is the same as the format
   * used by the method as_string().
   * 
   * @param os The std::ostream instance where to print the value.
   * @param ip The IPv4 instance to print in the os instance.
   */
  friend std::ostream& operator<<(std::ostream&, const IPv4&);

  /**
   * @brief >> operator to read an IPv4 instance from a std::iestream.
   * @details The format in with the value must be represented in the std::iestream
   * is the same as the format required in the std::string constructor.
   * 
   * @param is The std::iestream used to read the new IPv4 instance to create.
   * @param ip The new instance to set with the value read from is.
   */
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