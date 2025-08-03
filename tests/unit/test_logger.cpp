/**
 * @file test_logger.cpp
 * @brief Unit tests for the Logger system
 */

#include "logger.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

class LoggerTest {
public:
    static void test_logger_initialization() {
        std::cout << "Testing logger initialization..." << std::endl;
        
        Logger& logger = Logger::getInstance();
        logger.initialize(LogLevel::DEBUG, LogTarget::FILE, "test_logs/test.log");
        
        // Test log level setting
        assert(logger.getLogLevel() == LogLevel::DEBUG);
        
        // Test basic logging
        logger.log(LogLevel::INFO, "TEST", "Test initialization message");
        
        // Flush to ensure file is written
        logger.flush();
        
        // Check if log file was created
        assert(std::filesystem::exists("test_logs/test.log"));
        
        std::cout << "✅ Logger initialization test passed" << std::endl;
    }
    
    static void test_module_logger() {
        std::cout << "Testing module logger..." << std::endl;
        
        ModuleLogger test_logger("UNITTEST");
        
        // Test different log levels
        test_logger.debug("Debug message test");
        test_logger.info("Info message test");
        test_logger.warn("Warning message test");
        test_logger.error("Error message test");
        
        Logger::getInstance().flush();
        
        std::cout << "✅ Module logger test passed" << std::endl;
    }
    
    static void test_log_levels() {
        std::cout << "Testing log level filtering..." << std::endl;
        
        Logger& logger = Logger::getInstance();
        
        // Set to WARN level
        logger.setLogLevel(LogLevel::WARN);
        assert(logger.getLogLevel() == LogLevel::WARN);
        
        ModuleLogger test_logger("LEVEL_TEST");
        
        // These should not appear in output (below WARN level)
        test_logger.debug("This debug message should not appear");
        test_logger.info("This info message should not appear");
        
        // These should appear
        test_logger.warn("This warning should appear");
        test_logger.error("This error should appear");
        test_logger.critical("This critical message should appear");
        
        logger.flush();
        
        std::cout << "✅ Log level filtering test passed" << std::endl;
    }
    
    static void test_performance_logging() {
        std::cout << "Testing performance logging macros..." << std::endl;
        
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
        
        // Test performance logging macros
        PERF_LOG_START("UNITTEST", test_operation);
        
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        PERF_LOG_END("UNITTEST", test_operation);
        
        Logger::getInstance().flush();
        
        std::cout << "✅ Performance logging test passed" << std::endl;
    }
    
    static void cleanup() {
        Logger::getInstance().shutdown();
        
        // Clean up test files
        try {
            if (std::filesystem::exists("test_logs")) {
                std::filesystem::remove_all("test_logs");
            }
        } catch (const std::exception& e) {
            std::cerr << "Cleanup warning: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::cout << "🧪 Running Logger Unit Tests" << std::endl;
    std::cout << "=============================" << std::endl;
    
    try {
        LoggerTest::test_logger_initialization();
        LoggerTest::test_module_logger();
        LoggerTest::test_log_levels();
        LoggerTest::test_performance_logging();
        
        std::cout << std::endl;
        std::cout << "🎉 All logger unit tests passed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        LoggerTest::cleanup();
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        LoggerTest::cleanup();
        return 1;
    }
    
    LoggerTest::cleanup();
    return 0;
}