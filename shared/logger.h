#include <mutex>

#ifndef LOG_LEVEL
#define LOG_LEVEL 4 // default to info
#endif

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

const char* logLevelToString(LogLevel level)
{
    switch (level)
    {
        default: return "[Unknown]";
        case LogLevel::Trace: return "[Trace]";
        case LogLevel::Debug: return "[Debug]";
        case LogLevel::Info: return "[Info] ";
        case LogLevel::Warning: return "[Warn] ";
        case LogLevel::Error: return "[Error]";
        case LogLevel::Fatal: return "[Fatal]";
    }
}

const char* logLevelToColor(LogLevel level)
{
    switch (level)
    {
        default: return "\033[0m";
        case LogLevel::Trace: return "\033[37m";
        case LogLevel::Debug: return "\033[36m";
        case LogLevel::Info: return "\033[32m";
        case LogLevel::Warning: return "\033[33m";
        case LogLevel::Error: return "\033[31m";
        case LogLevel::Fatal: return "\033[31;1m";
    }
}

// Macros to log to console and file.
// If LOG_TO_FILE is defined, logs to file as well as console.
// If not defined, only logs to console.
// Only logs if the log level is greater than or equal to the current log level.

#if (LOG_LEVEL <= LOG_LEVEL_TRACE) && defined(LOG_TO_FILE)
    #define LOG_TRACE(message, ...) Logger::log(LogLevel::Trace, message, ##__VA_ARGS__)
#elif (LOG_LEVEL <= LOG_LEVEL_TRACE)
    #define LOG_TRACE(message, ...) Logger::log_to_console(LogLevel::Trace, message, ##__VA_ARGS__)
#else
    #define LOG_TRACE(message, ...)
#endif

#if (LOG_LEVEL <= LOG_LEVEL_DEBUG) && defined(LOG_TO_FILE)
    #define LOG_DEBUG(message, ...) Logger::log(LogLevel::Debug, message, ##__VA_ARGS__)
#elif (LOG_LEVEL <= LOG_LEVEL_DEBUG)
    #define LOG_DEBUG(message, ...) Logger::log_to_console(LogLevel::Debug, message, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(message, ...)
#endif

#if (LOG_LEVEL <= LOG_LEVEL_INFO) && defined(LOG_TO_FILE)
    #define LOG_INFO(message, ...) Logger::log(LogLevel::Info, message, ##__VA_ARGS__)
#elif (LOG_LEVEL <= LOG_LEVEL_INFO)
    #define LOG_INFO(message, ...) Logger::log_to_console(LogLevel::Info, message, ##__VA_ARGS__)
#else
    #define LOG_INFO(message, ...)
#endif

#if (LOG_LEVEL <= LOG_LEVEL_WARNING) && defined(LOG_TO_FILE)
    #define LOG_WARN(message, ...) Logger::log(LogLevel::Warning, message, ##__VA_ARGS__)
#elif (LOG_LEVEL <= LOG_LEVEL_WARNING)
    #define LOG_WARN(message, ...) Logger::log_to_console(LogLevel::Warning, message, ##__VA_ARGS__)
#else
    #define LOG_WARN(message, ...)
#endif

#if (LOG_LEVEL <= LOG_LEVEL_ERROR) && defined(LOG_TO_FILE)
    #define LOG_ERROR(message, ...) Logger::log(LogLevel::Error, message, ##__VA_ARGS__)
#elif (LOG_LEVEL <= LOG_LEVEL_ERROR)
    #define LOG_ERROR(message, ...) Logger::log_to_console(LogLevel::Error, message, ##__VA_ARGS__)
#else
    #define LOG_ERROR(message, ...)
#endif

#if (LOG_LEVEL <= LOG_LEVEL_FATAL) && defined(LOG_TO_FILE)
    #define LOG_FATAL(message, ...) Logger::log(LogLevel::Fatal, message, ##__VA_ARGS__)
#elif (LOG_LEVEL <= LOG_LEVEL_FATAL)
    #define LOG_FATAL(message, ...) Logger::log_to_console(LogLevel::Fatal, message, ##__VA_ARGS__)
#else
    #define LOG_FATAL(message, ...)
#endif

#if defined(LOG_TO_FILE)
    #define SHUTDOWN_LOG() Logger::shutdown()
#else
    #define SHUTDOWN_LOG()
#endif

class Logger
{
private:
    // Mutex for thread safety.
    static std::mutex m_console_mutex;
    static std::mutex m_file_mutex;

    // File to log to.
    static FILE* m_file;

    static std::tm* log_timestamp() {
        // log current time with trailing tab
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_c);
        return now_tm;
    }

    static void reset_ansi_color() {
        printf("\033[0m");
    }

public:

    template<typename... Args>
    static void log_to_file(LogLevel level, const char* message, Args... args)
    {
        std::scoped_lock lock(m_file_mutex);

        // if file is not open, open it in append mode
        if (m_file == nullptr) {
            #ifdef LOG_FILE
                m_file = fopen(LOG_FILE, "a");
            #else
                m_file = fopen("log.txt", "a");
            #endif
        }

        fprintf(m_file, "%s ", logLevelToString(level));
        auto now_tm = log_timestamp();

        fprintf(m_file, "(%d-%02d-%02d %02d:%02d:%02d):\t",
               now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday,
               now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec
        );

        fprintf(m_file, message, args...);
        fprintf(m_file, "\n");
        fflush(m_file);
    }

    template<typename... Args>
    static void log_to_console(LogLevel level, const char* message, Args... args) {
        std::scoped_lock lock(m_console_mutex);

        auto color = logLevelToColor(level);

        printf("%s%s ", color, logLevelToString(level));
        auto now_tm = log_timestamp();
        printf("(%d-%02d-%02d %02d:%02d:%02d):\t",
               now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday,
               now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec
        );
        printf(message, args...);
        printf("\n");
        reset_ansi_color();
    }

    template<typename... Args>
    static void log(LogLevel level, const char* message, Args... args)
    {
        log_to_console(level, message, args...);
        log_to_file(level, message, args...);
    }

    static void shutdown() {
        if (m_file != nullptr) {
            fclose(m_file);
            m_file = nullptr;
        }
    }
};

std::mutex Logger::m_console_mutex;
std::mutex Logger::m_file_mutex;

FILE* Logger::m_file = nullptr;