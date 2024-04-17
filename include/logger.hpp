#ifndef S_LOGGER_HPP__
#define S_LOGGER_HPP__

#include "color.hpp"
#include <stdio.h>
#include <string>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SLog(format, ...)                                                      \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::trace, __func__,     \
                                   format, ##__VA_ARGS__);                     \
    }

#define STrace(format, ...)                                                    \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::trace, __func__,     \
                                   format, ##__VA_ARGS__);                     \
    }

#define SInfo(format, ...)                                                     \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::info, __func__,      \
                                   format, ##__VA_ARGS__);                     \
    }

#define SNotice(format, ...)                                                   \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::notice, __func__,    \
                                   format, ##__VA_ARGS__);                     \
    }

#define SWarn(format, ...)                                                     \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::warning, __func__,   \
                                   format, ##__VA_ARGS__);                     \
    }

#define SError(format, ...)                                                    \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::error, __func__,     \
                                   format, ##__VA_ARGS__);                     \
    }

#define SCritical(format, ...)                                                 \
    {                                                                          \
        Serial::Logger::Log::__log(Serial::Logger::Level::critical, __func__,  \
                                   format, ##__VA_ARGS__);                     \
    }
namespace Serial {
namespace Logger {
enum Level { trace, info, notice, warning, error, critical, __numOfLevel };

static std::string LevelString[Level::__numOfLevel] = {"TRAC", "INFO", "NOTI",
                                                       "WARN", "ERRR", "CRIT"};

class Log {
  public:
    Log() {}
    ~Log() {}

  private:
    static void __time(FILE *__out) {
        time_t timer;
        char buffer[26];
        struct tm *tm_info;

        timer = time(NULL);
        tm_info = localtime(&timer);

        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(__out, "%s", buffer);
    }

    static void __prefix(FILE *__out, Level __level, const char *__func_name) {
        fprintf(__out, "[%s] ", LevelString[__level].c_str());
        __time(__out);
        fprintf(__out, " %s[%d:%d] ", __func_name, getppid(), getpid());
    }

    static void __suffix(FILE *__out) { fprintf(__out, "\n"); }

  public:
    template <typename... Args>
    static void __log(Level __level, const char *__func_name,
                      const char *__format, Args const &...__args) {
        FILE *__out = stdout;
        switch (__level) {
        case trace:
            __out = stdout;
            Appearance::ForegroundColor::Default.set(__out);
            break;
        case info:
            __out = stdout;
            Appearance::ForegroundColor::Cyan.set(__out);
            break;
        case notice:
            __out = stdout;
            Appearance::ForegroundColor::Cyan.set(__out);
            break;
        case warning:
            __out = stderr;
            Appearance::ForegroundColor::Yellow.set(__out);
            break;
        case error:
            __out = stdout;
            Appearance::ForegroundColor::Red.set(__out);
            break;
        case critical:
            __out = stdout;
            Appearance::ForegroundColor::Red.set(__out);
            Appearance::Decoration::Reverse.set(__out);
            break;
        case __numOfLevel:
            break;
        }

        __prefix(__out, __level, __func_name);
        fprintf(__out, __format, __args...);
        __suffix(__out);

        Appearance::ForegroundColor::Default.set(__out);
        Appearance::BackgroundColor::Default.set(__out);
        Appearance::Decoration::Default.set(__out);
    }
};

}; // namespace Logger
}; // namespace Serial

#endif