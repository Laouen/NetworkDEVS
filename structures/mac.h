#if !defined mac_h
#define mac_h

#include <cstdint> // Allows using ushort
#include <fstream>
#include <string>
#include <cstring> // memcpy
#include <sstream>
#include <iostream>

#define BROADCAST_MAC_ADDRESS std::string("FF:FF:FF:FF:FF:FF")

/**
 * @author Laouen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @struct MAC mac.h
 * 
 * @brief This struct declares a MAC type for  NetworksDEVS simulation purposes.
 * @details It's not meant to be a full MAC type and it does not implement any 
 * low leve MAC implementation. This struct represent a MAC Address as a 
 * unsigned short array of length 6 and it was designed to easely simulate 
 * networks using the NetworkDEVS framework. 
 */
struct MAC {
	ushort addr[6];

  /**
   * @brief Default constructor
   * @details This method construct a new MAC instance with value 0:0:0:0:0:0
   */
  MAC() {
    for(int i=0; i<6; ++i)
      addr[i] = 0;
  }

  /**
   * @brief Constructor from std::String
   * @details It construct a new MAC instance using the std::string >> operator 
   * to read values with string format 'a:b:c:d:e:f' where 0 <= a,b,c,d,e,f <= 255
   * in hexadecimal representation.
   * 
   * @param other_mac A string representing the MAC to construct.
   */
  MAC(const std::string& other_mac) {
    char ch;
    std::istringstream strm(other_mac);
    strm >> std::hex >> addr[0] >> ch >> std::hex >> addr[1] >> ch >> std::hex >> addr[2] >> ch; 
    strm >> std::hex >> addr[3] >> ch >> std::hex >> addr[4] >> ch >> std::hex >> addr[5];
  }

  /**
   * @brief Constructor from const char *
   * @details This constructor uses a const char * as the representation where 
   * to interpreat the MAC value to create.
   * 
   * @param other_mac A const char * that points to a representation of the MAC 
   * in the next form: 'a:b:c:d:e:f' where 0 <= a,b,c,d,e,f <= 255 in hexadecimal 
   * representation. 
   * This constructor fails if the pointer is not pointing to six consecutives 
   * unsigned short values and the new instance is created with trash values.
   */
  MAC(const char* other_mac) {
    char ch;
    std::istringstream strm(other_mac);
    strm >> std::hex >> addr[0] >> ch >> std::hex >> addr[1] >> ch >> std::hex >> addr[2] >> ch; 
    strm >> std::hex >> addr[3] >> ch >> std::hex >> addr[4] >> ch >> std::hex >> addr[5];
  }

  /**
   * @brief Constructor from unsigned short *
   * @details This constructor takes an unsigned short * and reads from it six
   * consecutives usigned short a,b,c,d,e and f to construct a new MAC with 
   * value a:b:c:d:e:f.
   * 
   * @param other_mac An unsigned short pointer that must point to six 
   * consecutives unsigned short values used to construct the new MAC instance. 
   * This constructor fails if the pointer is not pointing to six consecutives 
   * unsigned short values and the new instance is created with trash values.
   */
  MAC(ushort* other_mac) {
    for(int i=0; i<6; ++i)
      addr[i] = other_mac[i];
  }

  /**
   * @brief Copy constructor
   * @details This constructor uses an MAC instance to construct a copy of its
   * value. This constructor is the one used by C++ in the MAC default == operator.
   * 
   * @param other_mac A MAC instance to be copied.
   */
  MAC(const MAC& other_mac) {
    for(int i=0; i<6; ++i)
      addr[i] = other_mac.addr[i];
  }

  /**
   * @brief Equal comparator operator == against other MAC instance
   * @details This operator checks if the implicit MAC instance and the other
   * instance passed as parameter are equals or not. Two MAC instances 
   * are equals when their six unsigned short values are equals.
   * 
   * @param other The MAC instance to compare the implicit this MAC instance.
   * @return True if both other and the this instance are equals, False otherwise.
   */
  bool operator==(const MAC& other) const {
    bool res = true;
    for(int i=0; i<6; ++i)
      res = res && addr[i] == other.addr[i];
    return res;
  }

  /**
   * @brief Equal comparator operator == against a std::string
   * @details This operator checks whether the implicit MAC instance and the MAC 
   * value represented by the std::string passed as parameter are equals or not. 
   * To do so, it first construct a new MAC instance using the std::string
   * constructor and after that it compare the two MAC instances using 
   * the == operator with them.
   * 
   * @param other_string A string with a MAC representation that allows to construct 
   * a new MAC instance to be compared against the implicit this MAC instance.
   * @return True if both the represented MAC by other and the this instance 
   * are equals, False otherwise.
   */
  bool operator==(const std::string& other_string) const {
    MAC other = other_string;
    return *this == other;
  }

  /**
   * @brief Lower than comparator
   * @details This operator compares the implicit this instance agains the other
   * instances passed as parameter to return whether the this is lower than 
   * the other or not. A MAC A is lower than another MAC B when the first 
   * different value from left to right is lower in the A instance than in the B
   * instance. 
   * 
   * @param other The instance against what to compare the implicite instance.  
   * @return True if this is lower than other, False otherwise.
   */
  bool operator<(const MAC& other) const {
    for(int i=0; i<6; ++i) {
      if (addr[i] < other.addr[i])
        return true;
      else if (addr[i] > other.addr[i])
        return false;
    }
    return false;
  }

  /**
   * @brief Lower than comparator
   * @details This operator compares the implicit this instance agains the MAC 
   * value represented by the std::string other passed as parameter to return 
   * whether the this is lower than the other or not. To do so, it first 
   * construct a new MAC instance using the std::string constructor and after
   * that it compares the two instances.
   * 
   * @param other_string A string with a MAC representation that allows to construct 
   * a new MAC instance to be compared against the implicit this MAC instance.  
   * @return True if this is lower than other, False otherwise.
   */
  bool operator<(const std::string& other_string) const {
    MAC other = other_string;
    return *this < other; 
  }

  /**
   * @brief Returns a string formated as 'a:b;c:d:e:f' with the MAC value of the 
   * implicit this. where a,b,c,d,e and f are printed in hexadecimal format.
   * 
   * @details This method can be used to print the MAC value, the format used
   * to print the value in a string is consistent with the format expected by
   * the std::string constructor. The
   * @return An std::string with the formated value.
   */
  std::string as_string() const {
    std::stringstream ss;
    for (int i=0; i<6; ++i) {
      ss << std::hex << addr[i];
      if(i<5) ss << ":";
    }
    return ss.str();
  }

  /**
   * @brief Bitwise & operator
   * @details This operator implement a bitwise & operation between the value of
   * the implicit this instance and the other instance passed as parameter and 
   * returns the value of the operation. The this instances is not modified.
   * Example: FF:0:FF:1:F0:0F & 0:AD.FF.2:0F:F0 = 0:0:FF:0:0:0
   * 
   * @param other A MAC instance.
   * @return A MAC instance with the result of this & other.
   */
  MAC operator&(const MAC& other) const {
    MAC res;
    for(int i=0; i<6; ++i)
      res.addr[i] = addr[i] & other.addr[i];
    return res;
  }

  /**
   * @brief Bitwise | operator
   * @details This operator implement a bistwise | operation between the value of
   * the implicit this instance and the other instance passed as parameter nad 
   * returns the value of the operation. The this instance is not modified.
   * Example FF:0:FF:1:F0:0F | 0:AD.FF.2:0F:F0 = FF:AD:FF:3:FF:FF
   * 
   * @param other A MAC instance 
   * @return A MAC instance with the result of this | other.
   */
  MAC operator|(const MAC& other) const {
    MAC res;
    for(int i=0; i<6; ++i)
      res.addr[i] = addr[i] | other.addr[i];
    return res;
  }

  /**
   * @brief << operator to print the MAC value in a std::ostream
   * @details The format used to print the MAC value is the same as the format
   * used by the method as_string().
   * 
   * @param os The std::ostream instance where to print the value.
   * @param ip The MAC instance to print in the os instance.
   */
  friend std::ostream& operator<<(std::ostream&, const MAC&);

  /**
   * @brief >> operator to read an MAC instance from a std::iestream.
   * @details The format in with the value must be represented in the std::iestream
   * is the same as the format required in the std::string constructor.
   * 
   * @param is The std::iestream used to read the new MAC instance to create.
   * @param ip The new instance to set with the value read from is.
   */
  friend std::istream& operator>>(std::istream&, MAC&);
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