#include <iostream>
#include <iomanip>

#include <unistd.h>        // for gethostname()

#include <mpi.h>

#include "LogMessage.h"


using namespace mirabella;

LogMessage::LogLevel LogMessage::m_loglevel = LogMessage::LogLevel::LOG_INFO;
std::string LogMessage::m_hostname = LogMessage::set_hostname();

void LogMessage::setLogLevel_FATAL()    { LogMessage::setLogLevel(LogLevel::LOG_FATAL); }
void LogMessage::setLogLevel_ERROR()    { LogMessage::setLogLevel(LogLevel::LOG_ERROR); }
void LogMessage::setLogLevel_WARN()     { LogMessage::setLogLevel(LogLevel::LOG_WARN); }
void LogMessage::setLogLevel_INFO()     { LogMessage::setLogLevel(LogLevel::LOG_INFO); }
void LogMessage::setLogLevel_DEBUG_SHALLOW() { LogMessage::setLogLevel(LogLevel::LOG_DEBUG_SHALLOW); }
void LogMessage::setLogLevel_DEBUG_DEEP()    { LogMessage::setLogLevel(LogLevel::LOG_DEBUG_DEEP); }
void LogMessage::setLogLevel_DEBUG_XTREME()  { LogMessage::setLogLevel(LogLevel::LOG_DEBUG_XTREME); }


LogMessage::LogLevel LogMessage::getLogLevel() const { return m_loglevel; }
void LogMessage::setLogLevel(LogLevel level) { m_loglevel = level; }

const std::string LogMessage::loglevel2string(LogMessage::LogLevel level) const
{
    switch(level) {
        case LogMessage::LogLevel::LOG_FATAL : return "LOG_FATAL";
        case LogMessage::LogLevel::LOG_ERROR : return "LOG_ERROR";
        case LogMessage::LogLevel::LOG_WARN : return "LOG_WARN";
        case LogMessage::LogLevel::LOG_INFO : return "LOG_INFO";
        case LogMessage::LogLevel::LOG_DEBUG_SHALLOW : return "LOG_DEBUG_SHALLOW";
        case LogMessage::LogLevel::LOG_DEBUG_DEEP : return "LOG_DEBUG_DEEP";
        case LogMessage::LogLevel::LOG_DEBUG_XTREME : return "LOG_DEBUG_XTREME";
    };
}

std::string LogMessage::set_hostname()
{
    char hn[1024];
    int result = gethostname(hn, 1024);
    if(result != 0) {
        // TODO
    }

    return hn;
}

std::thread::id LogMessage::get_tid() const
{
    std::thread::id this_id = std::this_thread::get_id();
    return this_id;
}

std::string LogMessage::get_timestamp() const
{
    std::string time_str;
    time_t raw_time;
    time(&raw_time);
    time_str = ctime(&raw_time);
    return time_str.substr( 0 , time_str.size() - 1 );
}

std::string LogMessage::get_header()
{
    std::stringstream header;
    int tid;
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);

    header << "[" << get_timestamp() <<  ": " << clock() << "]";  
    header << "[" << m_hostname << "]"; 
    header << "[RANK=" << tid << "]"; 

// Don't display C++ Thread info as we're single threaded now
// and C++ thread ids suck
//  header << "[TID="  << get_tid() << "]"; 

    return header.str();
}

void LogMessage::compose()
{
    return;
}    

#ifdef DICK
template<typename ... Args>
LogMessage::LogMessage(LogLevel level, Args ... args)
{
    ss << get_header();
    ss << "<" << loglevel2string(level) << "> ";
    compose(args ...); 
}

template<typename First, typename ... Args>
void LogMessage::compose(First arg1, Args ... args)
{
    ss << " " << arg1;
    compose(args ...); 
}
#endif


LogMessage::LogMessage(LogMessage&& message) 
{
    ss.clear();
    ss.str(message.str());
    message.ss.clear();
}

LogMessage& LogMessage::operator=(LogMessage&& rhs)
{
    if(this != &rhs)
    {
        ss.clear();
        ss.str(rhs.str());
        rhs.ss.clear();
    } 
    return *this;
}

std::string LogMessage::str()
{
    return ss.str();
}

