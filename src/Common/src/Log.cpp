

#include "Log.hpp"
#include "ThreadLogger.hpp"

void _WriteLog(std::string data)
{
    g_logger->WriteLog(data);
}

