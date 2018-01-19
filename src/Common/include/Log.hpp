

#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <iostream>
#include <string>
#include <sstream>
#include <stdint.h>

class ThreadLogger;
extern  ThreadLogger * g_logger;

void WriteLog(std::string data);

        
#define debug_log(data)\
    {\
        std::stringstream ss;\
        ss << std::boolalpha << data << " [DEBUG] ["\
           << __FILE__ << ":"<< __LINE__<< ":" << __FUNCTION__ << "]\n";\
        WriteLog(ss.str());\
}\

#define warn_log(data)\
    {\
        std::stringstream ss;\
        ss << std::boolalpha << data << " [WARN] ["\
           << __FILE__ << ":"<< __LINE__<< ":" << __FUNCTION__ << "]\n";\
        WriteLog(ss.str());\
}\

#define error_log(data)\
    {\
        std::stringstream ss;\
        ss << std::boolalpha << data << " [ERROR] ["\
           << __FILE__ << ":"<< __LINE__<< ":" << __FUNCTION__ << "]\n";\
        WriteLog(ss.str());\
}\

#define trace_log(data)\
    {\
        std::stringstream ss;\
        ss << std::boolalpha << data << " [TRACE] ["\
           << __FILE__ << ":"<< __LINE__<< ":" << __FUNCTION__ << "]\n";\
        WriteLog(ss.str());\
}\

#define info_log(data)\
    {\
        std::stringstream ss;\
        ss << std::boolalpha << data << " [INFO] ["\
           << __FILE__ << ":"<< __LINE__<< ":" << __FUNCTION__ << "]\n";\
        WriteLog(ss.str());\
}\

#endif


