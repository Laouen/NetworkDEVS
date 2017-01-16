#if !defined parser_h
#define parser_h

#include "simulator.h"
#include "logger.h"

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

template<class INPUT>
class Parser {
private:
  std::ifstream file;
  Logger logger;

public:
  // Constructors and destructors
  Parser() {
    logger.setModuleName("Parser");
  }

  Parser(const char* file_path) {
    logger.setModuleName("Parser");
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
      logger.info("Input file opened."); // TODO put file_path
      return true;
    }
    logger.error("Faild opening file "); // TODO put file_path
    return false;
  }

  bool file_open() {
    return file.is_open();
  }

  std::pair<double,INPUT> next_input() {
    INPUT result;
    double next_time;

    if (file.eof()) {
      logger.info("End of file.");
      throw std::exception();
    }

    file >> next_time;
    file >> result;

    return std::make_pair(next_time,result);
  }
};

#endif