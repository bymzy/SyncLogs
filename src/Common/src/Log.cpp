

#include "Log.hpp"
#include "ThreadLogger.hpp"

void WriteLog(std::string data)
{
    g_logger->WriteLog(data);
}

