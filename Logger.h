#pragma once

#include <string>
#include <fstream>

#include <mpi.h>

#include "LogMessage.h"

namespace mirabella {

class Logger
{
public:
    explicit Logger(int id = 0);

    void Log(LogMessage& message);

    void Log(LogMessage message);

private:
    int l_id;

    void send_message(std::string message); 

};


static Logger* L;

#ifdef USE_LOGGING

#define Log_FATAL(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_FATAL, MSG)) 
#define Log_ERROR(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_ERROR, MSG)) 
#define Log_WARN(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_WARN, MSG)) 
#define Log_INFO(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_INFO, MSG)) 
#define Log_DEBUG_SHALLOW(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_DEBUG_SHALLOW, MSG)) 
#define Log_DEBUG_DEEP(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_DEBUG_DEEP, MSG)) 
#define Log_DEBUG_XTREME(MSG) L->Log(LogMessage(LogMessage::LogLevel::LOG_DEBUG_XTREME, MSG)) 

#else

#define Log_FATAL(MSG) 
#define Log_ERROR(MSG)
#define Log_WARN(MSG)
#define Log_INFO(MSG)
#define Log_DEBUG_SHALLOW(MSG) 
#define Log_DEBUG_DEEP(MSG) 
#define Log_DEBUG_XTREME(MSG) 


#endif // USE_LOGGING

class LogWriter
{
public:
    explicit LogWriter(std::string logfile);

    ~LogWriter();

private:
    std::string    lw_filename;
    std::ofstream       lw_ofstream;
    char*          lw_buffer;

    void process_messages();

};

};  // namespace mirabella
