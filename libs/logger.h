#if !defined Logger_h
#define Logger_h

//#define show_log
#define show_info
//#define show_debug
#define show_error

#include <string>
#include "simulator.h"

class Logger {
private:
	std::string module_name = "";
public:
	Logger() {};
	Logger(std::string other_module_name) {
		module_name = other_module_name;
	}

	void setModuleName(std::string other_module_name) {
		module_name = other_module_name;
	}

	void log(std::string msg) const {
		#ifdef show_log
		printLog("[LOG] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}

	void info(std::string msg) const {
		#ifdef show_info
		printLog("[INFO] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}

	void debug(std::string msg) const {
		#ifdef show_debug
		printLog("[DEBUG] - ");
		printLog(("["+module_name+"] ").c_str());
		printLog(msg.c_str());
		printLog("\n");
		#endif
	}

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