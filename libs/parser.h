#if !defined parser_h
#define parser_h

#include <stddef.h>
#include <cstdint> // Allows to use ushort

#include <string>
#include <fstream>

/**
 * 
 * @author Laouen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @brief A Parser that reads inputs of type INPUT from files, construct the 
 * corresponding INPUT instance and returns it. 
 *  
 * @details The Parsers uses the >> operator and the INPUT type must implement 
 * this operator, otherwise it fails in compile time. 
 * The Parser reads one input for each line of the file, thus, the >> operator
 * must parse a valid instance from a single line, otherwise it fails in runtime. 
 * The file can include at the begining of each line a double presition floating 
 * number and ask the parser to read that first number as a simulation virtual 
 * time to asociate with the input.
 * 
 * @tparam INPUT Data type of the input to parse from the file.
 */
template<class INPUT>
class Parser {
private:
  std::ifstream file;

public:
  
  /**
   * @brief Default constructor.
   * 
   * @details This constructor does not open any file and a file must be opened
   * before trying to read any input.
   */
  Parser() {
  }

  /**
   * @brief Constructor that opens a file
   * 
   * @details This constructor initializes a parser instance opening a file and
   * if this contructor is used, the parser is ready to read input unless the 
   * open file process fails. 
   * 
   * @param file_path A const char * with the path of the file to open.
   */
  Parser(const char* file_path) {
    this->open_file(file_path);
  }

  /**
   * @brief Destructor of the class.
   * 
   * @details This method is called when the instance is destroyed and if the
   * there is a opened file, it closes the file.
   */
  ~Parser() {
    if (file.is_open()) {
      file.close();
    }
  }

  /**
   * @brief Opens a file to read input from.
   * 
   * @param file_path A const char * with the path of the file to open.
   * @return True if the file was correctly opened, False otherwise.
   */
  bool open_file(const char* file_path) {
    file.open(file_path);
    if (file.is_open()) {
      return true;
    }
    return false;
  }

  /**
   * @brief Checks whether a file was correctly opened or not.
   * @details If this method returns False the methods next_timed_input and 
   * next_input can not be used.
   * 
   * @return True is a fila was correctly opened, False otherwise.
   */
  bool file_open() {
    return file.is_open();
  }

  /**
   * @brief Reads the next line of the file and parses the first value as the 
   * time and the res of the line as the INPUT.
   * 
   * @details This methods fails if the first element of the line is not a valid
   * double presition float value representation. The rest of the line must 
   * contain te whole INPUT representation to parse, if not the case, it also fails.
   * 
   * @return a std::pair<double,INPUT> where the first parameter is the parsed time
   * and the second parameter is the parsed input.
   */
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

  /**
   * @brief Reads the next line of the file and parses the whole line as the INPUT.
   * 
   * @details The line must contain the whole INPUT representation to parse, 
   * if not the case it fails.
   * 
   * @return An INPUT instance with the value of the parsed line.
   */
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