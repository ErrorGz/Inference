#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <mutex>
#include <thread>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <queue>
#include <condition_variable>
#include <atomic>

/**
 * @brief Industrial Logging System
 * 
 * Multi-level, multi-target logging system for industrial applications
 * Header-only implementation for easy integration
 */

// Log levels (ordered by severity)
enum class LogLevel {
    TRACE = 0,    // Detailed trace information
    DEBUG = 1,    // Debug information
    INFO = 2,     // General information
    WARN = 3,     // Warning messages
    ERROR = 4,    // Error messages
    CRITICAL = 5  // Critical system errors
};

// Log targets
enum class LogTarget {
    CONSOLE = 1,  // Console output
    FILE = 2,     // File output
    BOTH = 3      // Both console and file
};

/**
 * @brief Main Logger Class - Header-only implementation
 */
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    /**
     * @brief Initialize logger with configuration
     */
    void initialize(LogLevel log_level = LogLevel::INFO,
                   LogTarget log_target = LogTarget::BOTH,
                   const std::string& log_file_path = "inference_service.log",
                   size_t max_file_size_mb = 10,
                   size_t max_backup_files = 5) {
        if (!pImpl) {
            pImpl = std::make_unique<Impl>();
        }
        pImpl->initialize(log_level, log_target, log_file_path, max_file_size_mb, max_backup_files);
    }

    /**
     * @brief Log a message
     */
    void log(LogLevel level, const std::string& module, const std::string& message) {
        if (!pImpl) {
            pImpl = std::make_unique<Impl>();
            pImpl->initialize(LogLevel::INFO, LogTarget::CONSOLE, "inference_service.log", 10, 5);
        }
        pImpl->logMessage(level, module, message);
    }

    /**
     * @brief Set minimum log level
     */
    void setLogLevel(LogLevel level) {
        if (pImpl) {
            pImpl->current_log_level = level;
        }
    }

    /**
     * @brief Get current log level
     */
    LogLevel getLogLevel() const {
        return pImpl ? pImpl->current_log_level : LogLevel::INFO;
    }

    /**
     * @brief Flush all pending logs
     */
    void flush() {
        if (pImpl) {
            pImpl->flush();
        }
    }

    /**
     * @brief Shutdown logger (call before program exit)
     */
    void shutdown() {
        if (pImpl) {
            pImpl->shutdown();
        }
    }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    class Impl {
    public:
        LogLevel current_log_level = LogLevel::INFO;
        LogTarget log_target = LogTarget::BOTH;
        std::string log_file_path = "inference_service.log";
        size_t max_file_size_bytes = 10 * 1024 * 1024; // 10MB
        size_t max_backup_files = 5;
        
        std::ofstream log_file;
        std::mutex log_mutex;
        
        // Async logging
        std::queue<std::string> log_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_condition;
        std::thread logging_thread;
        std::atomic<bool> should_stop{false};
        
        Impl() = default;
        
        ~Impl() {
            shutdown();
        }
        
        void initialize(LogLevel log_level, LogTarget target, const std::string& file_path,
                       size_t max_file_size_mb, size_t max_backup_count) {
            current_log_level = log_level;
            log_target = target;
            log_file_path = file_path;
            max_file_size_bytes = max_file_size_mb * 1024 * 1024;
            max_backup_files = max_backup_count;
            
            // Open log file if needed
            if (log_target == LogTarget::FILE || log_target == LogTarget::BOTH) {
                openLogFile();
            }
            
            // Start async logging thread
            should_stop = false;
            logging_thread = std::thread(&Impl::loggingWorker, this);
            
            // Log initialization
            logMessage(LogLevel::INFO, "LOGGER", "Logging system initialized");
            logMessage(LogLevel::INFO, "LOGGER", "Log level: " + logLevelToString(log_level));
            logMessage(LogLevel::INFO, "LOGGER", "Log target: " + logTargetToString(target));
            if (log_target != LogTarget::CONSOLE) {
                logMessage(LogLevel::INFO, "LOGGER", "Log file: " + file_path);
            }
        }
        
        void openLogFile() {
            try {
                // Create directory if it doesn't exist
                std::filesystem::path file_path(log_file_path);
                std::filesystem::create_directories(file_path.parent_path());
                
                log_file.open(log_file_path, std::ios::app);
                if (!log_file.is_open()) {
                    std::cerr << "Failed to open log file: " << log_file_path << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception opening log file: " << e.what() << std::endl;
            }
        }
        
        void logMessage(LogLevel level, const std::string& module, const std::string& message) {
            if (level < current_log_level) {
                return;
            }
            
            std::string formatted_message = formatLogMessage(level, module, message);
            
            // Add to async queue
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                log_queue.push(formatted_message);
            }
            queue_condition.notify_one();
        }
        
        std::string formatLogMessage(LogLevel level, const std::string& module, const std::string& message) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;
            
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            ss << "." << std::setfill('0') << std::setw(3) << ms.count();
            ss << " [" << std::this_thread::get_id() << "]";
            ss << " [" << std::setw(8) << logLevelToString(level) << "]";
            ss << " [" << std::setw(15) << module << "] ";
            ss << message;
            
            return ss.str();
        }
        
        void loggingWorker() {
            while (!should_stop || !log_queue.empty()) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_condition.wait(lock, [this] { return !log_queue.empty() || should_stop; });
                
                while (!log_queue.empty()) {
                    std::string message = log_queue.front();
                    log_queue.pop();
                    lock.unlock();
                    
                    writeLogMessage(message);
                    
                    lock.lock();
                }
            }
        }
        
        void writeLogMessage(const std::string& message) {
            std::lock_guard<std::mutex> lock(log_mutex);
            
            // Write to console
            if (log_target == LogTarget::CONSOLE || log_target == LogTarget::BOTH) {
                std::cout << message << std::endl;
            }
            
            // Write to file
            if ((log_target == LogTarget::FILE || log_target == LogTarget::BOTH) && log_file.is_open()) {
                log_file << message << std::endl;
                log_file.flush();
                
                // Check if log rotation is needed
                if (needsRotation()) {
                    rotateLogFile();
                }
            }
        }
        
        bool needsRotation() {
            if (!log_file.is_open()) return false;
            
            try {
                auto file_size = std::filesystem::file_size(log_file_path);
                return file_size >= max_file_size_bytes;
            } catch (const std::exception&) {
                return false;
            }
        }
        
        void rotateLogFile() {
            log_file.close();
            
            try {
                // Rotate existing backup files
                for (int i = max_backup_files - 1; i >= 1; --i) {
                    std::string old_backup = log_file_path + "." + std::to_string(i);
                    std::string new_backup = log_file_path + "." + std::to_string(i + 1);
                    
                    if (std::filesystem::exists(old_backup)) {
                        if (i == max_backup_files - 1) {
                            std::filesystem::remove(old_backup);
                        } else {
                            std::filesystem::rename(old_backup, new_backup);
                        }
                    }
                }
                
                // Move current log to backup
                std::string first_backup = log_file_path + ".1";
                std::filesystem::rename(log_file_path, first_backup);
                
            } catch (const std::exception& e) {
                std::cerr << "Log rotation failed: " << e.what() << std::endl;
            }
            
            // Reopen log file
            openLogFile();
            logMessage(LogLevel::INFO, "LOGGER", "Log file rotated");
        }
        
        void flush() {
            // Wait for queue to be empty with timeout
            std::unique_lock<std::mutex> lock(queue_mutex);
            auto timeout = std::chrono::milliseconds(1000); // 1 second timeout
            queue_condition.wait_for(lock, timeout, [this] { return log_queue.empty() || should_stop; });
            
            std::lock_guard<std::mutex> file_lock(log_mutex);
            if (log_file.is_open()) {
                log_file.flush();
            }
        }
        
        void shutdown() {
            if (logging_thread.joinable()) {
                // First, flush any remaining messages
                flush();
                
                // Then signal shutdown
                should_stop = true;
                queue_condition.notify_all();
                
                // Wait for thread to finish with timeout
                if (logging_thread.joinable()) {
                    logging_thread.join();
                }
            }
            
            // Close file after thread is joined
            if (log_file.is_open()) {
                log_file.flush();
                log_file.close();
            }
        }
        
        std::string logLevelToString(LogLevel level) {
            switch (level) {
                case LogLevel::TRACE: return "TRACE";
                case LogLevel::DEBUG: return "DEBUG";
                case LogLevel::INFO: return "INFO";
                case LogLevel::WARN: return "WARN";
                case LogLevel::ERROR: return "ERROR";
                case LogLevel::CRITICAL: return "CRITICAL";
                default: return "UNKNOWN";
            }
        }
        
        std::string logTargetToString(LogTarget target) {
            switch (target) {
                case LogTarget::CONSOLE: return "CONSOLE";
                case LogTarget::FILE: return "FILE";
                case LogTarget::BOTH: return "BOTH";
                default: return "UNKNOWN";
            }
        }
    };

    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Module Logger - provides convenient logging interface for specific modules
 */
class ModuleLogger {
public:
    explicit ModuleLogger(const std::string& module_name) : module_name_(module_name) {}

    void trace(const std::string& message) {
        Logger::getInstance().log(LogLevel::TRACE, module_name_, message);
    }
    
    void debug(const std::string& message) {
        Logger::getInstance().log(LogLevel::DEBUG, module_name_, message);
    }
    
    void info(const std::string& message) {
        Logger::getInstance().log(LogLevel::INFO, module_name_, message);
    }
    
    void warn(const std::string& message) {
        Logger::getInstance().log(LogLevel::WARN, module_name_, message);
    }
    
    void error(const std::string& message) {
        Logger::getInstance().log(LogLevel::ERROR, module_name_, message);
    }
    
    void critical(const std::string& message) {
        Logger::getInstance().log(LogLevel::CRITICAL, module_name_, message);
    }

    // Template methods for formatted logging
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
    void warn(const std::string& format, Args... args) {
        log(LogLevel::WARN, format, args...);
    }

    template<typename... Args>
    void error(const std::string& format, Args... args) {
        log(LogLevel::ERROR, format, args...);
    }

    template<typename... Args>
    void critical(const std::string& format, Args... args) {
        log(LogLevel::CRITICAL, format, args...);
    }

private:
    std::string module_name_;

    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args... args) {
        std::stringstream ss;
        ((ss << args << " "), ...);
        Logger::getInstance().log(level, module_name_, format + " " + ss.str());
    }
};

// Utility macros for convenient logging
#define LOG_TRACE(module, msg) Logger::getInstance().log(LogLevel::TRACE, module, msg)
#define LOG_DEBUG(module, msg) Logger::getInstance().log(LogLevel::DEBUG, module, msg)
#define LOG_INFO(module, msg) Logger::getInstance().log(LogLevel::INFO, module, msg)
#define LOG_WARN(module, msg) Logger::getInstance().log(LogLevel::WARN, module, msg)
#define LOG_ERROR(module, msg) Logger::getInstance().log(LogLevel::ERROR, module, msg)
#define LOG_CRITICAL(module, msg) Logger::getInstance().log(LogLevel::CRITICAL, module, msg)

// Performance logging macros
#define PERF_LOG_START(module, operation) \
    auto start_time_##operation = std::chrono::high_resolution_clock::now(); \
    LOG_DEBUG(module, "Starting operation: " #operation)

#define PERF_LOG_END(module, operation) \
    auto end_time_##operation = std::chrono::high_resolution_clock::now(); \
    auto duration_##operation = std::chrono::duration_cast<std::chrono::microseconds>(end_time_##operation - start_time_##operation); \
    LOG_INFO(module, "Operation " #operation " completed in " + std::to_string(duration_##operation.count()) + " microseconds")