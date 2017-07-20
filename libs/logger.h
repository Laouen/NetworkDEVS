#if !defined Logger_h
#define Logger_h

#define show_log
#define show_info
//#define show_debug
#define show_error

#include <string>
#include "simulator.h"

/**
 * @author Laouen Louan Mayal Belloli
 * @date 14 May 2017
 * 
 * @class Logger logger.h
 * 
 * @brief This class print formated std::string in the pdevs.log file used by the
 * PowerDEVS simulator.
 * @details The logger divides the logs in four types: logs, info, debug and error.
 * - logs: Irrelevant for the model state in the simulation, but relevant for other purposes.
 * - info: Relevan information from the model states in the simulation.
 * - debug: Messages used to debug de model in runtime.
 * - error: Messages used to print runtime catched errors.
 * The meaning of each log kind can be gived by the modeler and it does not have 
 * to folow the previuos suggestion.
 * This allows the modeler to divide the messages to log in four kinds and to 
 * decide with kind of message. When running simulations it can turn on or off 
 * any the four kinds of log.
 * The logger also has a module name that is set in the constructor or after. In 
 * each log the logger also prints the log kind and the module name in order to 
 * better identify which model has printed each message. 
 * @return [description]
 */
class Logger {
private:
	std::string module_name = "";
public:

	/**
	 * @brief Default constructor
	 * @details It construct a new instance of Logger without module name.
	 */

	Logger() {};

	/**
	 * @brief Constructor with module name.
	 * @details This constructor takes as parameter a std::string that it is used
	 * to set the module name that will be printed with each log.
	 * 
	 * @param other_module_name A std::string with the module name that will 
	 * use this instance to print logs.
	 */
	Logger(std::string other_module_name) {
		module_name = other_module_name;
	}

	/**
	 * @brief It set a new module name
	 * @details This method allows to set the module name after the instance 
	 * construction or to modify the current module name of the instance
	 * 
	 * @param other_module_name A std::string with the new module name that will
	 * use this instance to print logs 
	 */
	void setModuleName(std::string other_module_name) {
		module_name = other_module_name;
	}

	/**
	 * @brief Prints log messages.
	 * @details If #define show_log is not commented, this method prints messages
	 * under the [LOG] tag. If #define show_log is commented, thos method does
	 * nothing. 
	 * 
	 * @param msg The std::string to print
	 */
	void log(std::string msg) const {
		#ifdef show_log
		printLog("[LOG] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}

	/**
	 * @brief Prints info messages.
	 * @details If #define show_info is not commented, this method prints messages
	 * under the [INFO] tag. If #define show_info is commented, thos method does
	 * nothing. 
	 * 
	 * @param msg The std::string to print
	 */
	void info(std::string msg) const {
		#ifdef show_info
		printLog("[INFO] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}

	/**
	 * @brief Prints info messages.
	 * @details If #define show_debug is not commented, this method prints messages
	 * under the [DEBUG] tag. If #define show_debug is commented, thos method does
	 * nothing. 
	 * 
	 * @param msg The std::string to print
	 */
	void debug(std::string msg) const {
		#ifdef show_debug
		printLog("[DEBUG] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}

	/**
	 * @brief Prints info messages.
	 * @details If #define show_error is not commented, this method prints messages
	 * under the [ERROR] tag. If #define show_error is commented, thos method does
	 * nothing. 
	 * 
	 * @param msg The std::string to print
	 */
	void error(std::string msg) const {
		#ifdef show_error
		printLog("[ERROR] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}
};

#endif