#if !defined parser_h
#define parser_h

#include "simulator.h"

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

template<class INPUT>
class Parser {
private:
  std::ifstream file;

public:
  // Constructors and destructors
  Parser() {}

  Parser(const char* file_path) {
    this->open_file(file_path);
  }

  ~Parser() {
    if (file.is_open()) {
      file.close();
    }
  }

  bool open_file(const char* file_path) {
    file.open(file_path);
    if (file.is_open()) {
      printLog("[Parser] Input file ");
      printLog(file_path);
      printLog(" opened.\n");
      return true;
    }
    printLog("faild opening file ");
    printLog(file_path);
    printLog("\n");
    return false;
  }

  std::pair<double,INPUT> next_input() {
    INPUT result;
    double next_time;

    if (file.eof()) {
      printLog("[Parser] eof\n");
      throw std::exception();
    }

    file >> next_time;
    file >> result;

    return std::make_pair(next_time,result);
  }
};

#endif