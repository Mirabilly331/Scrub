#pragma once

#include <string>
#include <sstream>
#include <thread>

namespace mirabella
{

class LogMessage
{
public:
    enum class LogLevel
    {
        LOG_FATAL,
        LOG_ERROR,
        LOG_WARN,
        LOG_INFO,
        LOG_DEBUG_SHALLOW,
        LOG_DEBUG_DEEP,
        LOG_DEBUG_XTREME
    };

    void setLogLevel_FATAL();
    void setLogLevel_ERROR();
    void setLogLevel_WARN();
    void setLogLevel_INFO();
    void setLogLevel_DEBUG_SHALLOW();
    void setLogLevel_DEBUG_DEEP();
    void setLogLevel_DEBUG_XTREME();

    LogLevel getLogLevel() const;
    const std::string loglevel2string(LogMessage::LogLevel) const;

    std::string str();
#ifdef DICK
    template<typename ... Args>
    LogMessage(LogLevel level, Args ... args);
#else
    template<typename ... Args>
    LogMessage(LogLevel level, Args ... args)
    {
        ss << get_header();
        ss << "<" << loglevel2string(level) << "> ";
        compose(args ...);
    }
#endif

    LogMessage(const LogMessage& message) = delete;
    LogMessage& operator=(const LogMessage& rhs) = delete;

    LogMessage(LogMessage&& message);

    LogMessage& operator=(LogMessage&& rhs);

private:
    static LogLevel m_loglevel;
    static std::string m_hostname;

    std::stringstream ss;

    void compose();
#ifdef DICK
    template<typename First, typename ... Args>
    void compose(First arg1, Args ... args);
#else
template<typename First, typename ... Args>
void compose(First arg1, Args ... args)
{
    ss << " " << arg1;
    compose(args ...);
}

#endif

    void setLogLevel(const LogMessage::LogLevel level);

    std::string get_header();
    std::string get_timestamp() const;
    std::thread::id get_tid() const;

    static std::string set_hostname();
};

}; // namespace mirabella
