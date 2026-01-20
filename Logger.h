#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <vector>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#ifdef ERROR
#undef ERROR
#endif
#ifdef CRITICAL
#undef CRITICAL
#endif
#endif


// ==================== Класс для цветного вывода в консоль ====================
class ConsoleColor {
public:
    enum Code {
        RESET = 0,
        BOLD = 1,
        DIM = 2,
        UNDERLINE = 4,

        FG_RED = 31,
        FG_GREEN = 32,
        FG_YELLOW = 33,
        FG_BLUE = 34,
        FG_MAGENTA = 35,
        FG_CYAN = 36,
        FG_WHITE = 37,
        FG_DEFAULT = 39,

        BG_RED = 41,
        BG_GREEN = 42,
        BG_YELLOW = 43,
        BG_BLUE = 44,
        BG_MAGENTA = 45,
        BG_CYAN = 46,
        BG_WHITE = 47,
        BG_DEFAULT = 49
    };

    static std::string set(Code code) {
        return "\033[" + std::to_string(code) + "m";
    }

    static std::string reset() {
        return set(RESET);
    }
};

// ==================== Уровни логирования ====================
enum class LogLevel {
    TRACE,      // Самые подробные сообщения (отладка)
    DEBUG,      // Отладочные сообщения
    INFO,       // Информационные сообщения
    WARNING,    // Предупреждения
    ERROR,      // Ошибки
    CRITICAL    // Критические ошибки (приложение может завершиться)
};

// ==================== Базовый класс логгера ====================
class Logger {
public:
    Logger(const std::string& name) : name(name) {}
    virtual ~Logger() = default;

    // Основные методы логирования
    template<typename... Args>
    void trace(const std::string& format, Args... args) {
        log(LogLevel::TRACE, format, args...);
    }

    template<typename... Args>
    void debug(const std::string& format, Args... args) {
        log(LogLevel::DEBUG, format, args...);
    }

    template<typename... Args>
    void info(const std::string& format, Args... args) {
        log(LogLevel::INFO, format, args...);
    }

    template<typename... Args>
    void warning(const std::string& format, Args... args) {
        log(LogLevel::WARNING, format, args...);
    }

    template<typename... Args>
    void error(const std::string& format, Args... args) {
        log(LogLevel::ERROR, format, args...);
    }

    template<typename... Args>
    void critical(const std::string& format, Args... args) {
        log(LogLevel::CRITICAL, format, args...);
    }

    // Установка уровня логирования
    void setLevel(LogLevel level) { minLevel = level; }
    LogLevel getLevel() const { return minLevel; }

    // Получение имени логгера
    const std::string& getName() const { return name; }

protected:
    virtual void write(LogLevel level, const std::string& message) = 0;

private:
    std::string name;
    LogLevel minLevel = LogLevel::INFO;

    // Форматирование сообщения с поддержкой переменных
    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args... args) {
        if (level < minLevel) return;

        // Форматируем сообщение
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), format.c_str(), args...);

        write(level, std::string(buffer));
    }

    // Перегрузка для сообщений без форматирования
    void log(LogLevel level, const std::string& message) {
        if (level < minLevel) return;
        write(level, message);
    }
};

// ==================== Консольный логгер (цветной вывод) ====================
class ConsoleLogger : public Logger {
public:
    ConsoleLogger(const std::string& name) : Logger(name) {}

protected:
    void write(LogLevel level, const std::string& message) override {
        std::lock_guard<std::mutex> lock(mutex);

        // Получаем текущее время
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif

        // Форматируем время
        std::ostringstream timeStream;
        timeStream << std::put_time(&tm, "%H:%M:%S") << '.'
            << std::setfill('0') << std::setw(3) << ms.count();

        // Получаем цвет для уровня логирования
        std::string color = getLevelColor(level);
        std::string levelStr = getLevelString(level);

        // Выводим с цветами
        std::cout << "\033[36m[" << timeStream.str() << "] "
            << color << "[" << levelStr << "] "
            << "\033[37m[" << getName() << "] "
            << "\033[0m" << message
            << ConsoleColor::reset() << std::endl;
    }

private:
    std::mutex mutex;

    std::string getLevelColor(LogLevel level) {
        switch (level) {
        case LogLevel::TRACE:    return "\033[37m";
        case LogLevel::DEBUG:    return "\033[34m";
        case LogLevel::INFO:     return "\033[32m";
        case LogLevel::WARNING:  return "\033[33m";
        case LogLevel::ERROR:    return "\033[31m";
        case LogLevel::CRITICAL: return "\033[35m";
        default:                 return "\033[0m";
        }
    }

    std::string getLevelString(LogLevel level) {
        switch (level) {
        case LogLevel::TRACE:    return "TRACE";
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
        }
    }
};

// ==================== Файловый логгер ====================
class FileLogger : public Logger {
public:
    FileLogger(const std::string& name, const std::string& filename)
        : Logger(name), filename(filename) {
        file.open(filename, std::ios::app);
    }

    ~FileLogger() {
        if (file.is_open()) {
            file.close();
        }
    }

protected:
    void write(LogLevel level, const std::string& message) override {
        std::lock_guard<std::mutex> lock(mutex);

        if (!file.is_open()) return;

        // Получаем текущее время
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif

        // Форматируем время
        std::ostringstream timeStream;
        timeStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(3) << ms.count();

        // Получаем строку уровня
        std::string levelStr = getLevelString(level);

        // Записываем в файл
        file << "[" << timeStream.str() << "] "
            << "[" << levelStr << "] "
            << "[" << getName() << "] "
            << message << std::endl;

        file.flush();
    }

private:
    std::string filename;
    std::ofstream file;
    std::mutex mutex;

    std::string getLevelString(LogLevel level) {
        switch (level) {
        case LogLevel::TRACE:    return "TRACE";
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
        }
    }
};

// ==================== Менеджер логгеров ====================
class LogManager {
public:
    static LogManager& getInstance() {
        static LogManager instance;
        return instance;
    }

    // Создание логгеров
    Logger* createConsoleLogger(const std::string& name) {
        auto logger = std::make_unique<ConsoleLogger>(name);
        Logger* ptr = logger.get();
        loggers[name] = std::move(logger);
        return ptr;
    }

    Logger* createFileLogger(const std::string& name, const std::string& filename) {
        auto logger = std::make_unique<FileLogger>(name, filename);
        Logger* ptr = logger.get();
        loggers[name] = std::move(logger);
        return ptr;
    }

    // Получение логгера
    Logger* getLogger(const std::string& name) {
        auto it = loggers.find(name);
        if (it != loggers.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Удаление логгера
    void removeLogger(const std::string& name) {
        loggers.erase(name);
    }

    // Установка минимального уровня для всех логгеров
    void setGlobalLevel(LogLevel level) {
        for (auto& pair : loggers) {
            pair.second->setLevel(level);
        }
    }

private:
    LogManager() = default;
    ~LogManager() = default;

    std::unordered_map<std::string, std::unique_ptr<Logger>> loggers;
};

// ==================== Макросы для удобства ====================
// Быстрые макросы для глобального логгера
#define LOG_TRACE(...)      getEngineLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)      getEngineLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)       getEngineLogger()->info(__VA_ARGS__)
#define LOG_WARNING(...)    getEngineLogger()->warning(__VA_ARGS__)
#define LOG_ERROR(...)      getEngineLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)   getEngineLogger()->critical(__VA_ARGS__)

// Функция для получения логгера по умолчанию
inline Logger* getEngineLogger() {
    static Logger* logger = nullptr;
    if (!logger) {
        logger = LogManager::getInstance().createConsoleLogger("Engine");
        logger->setLevel(LogLevel::INFO);
    }
    return logger;
}