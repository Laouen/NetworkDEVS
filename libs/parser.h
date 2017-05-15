#if !defined parser_h
#define parser_h

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
  Parser() {
  }

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
      return true;
    }
    return false;
  }

  bool file_open() {
    return file.is_open();
  }

  std::pair<double,INPUT> next_timed_input() {
    INPUT result;
    double next_time;

    if (!file.is_open() || file.eof()) {
      throw std::exception();
    }

    file >> next_time;
    file >> result;

    return std::make_pair(next_time,result);
  }

  INPUT next_input() {
    INPUT result;

    if (file.eof()) {
      throw std::exception();
    }

    file >> result;

    return result;
  }
};

#endif