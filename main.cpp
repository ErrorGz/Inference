#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include "inference_service.hpp"
#include "logger.hpp"

// Global flag for graceful shutdown
std::atomic<bool> shutdown_requested(false);

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        shutdown_requested = true;
        std::cout << "\nShutdown signal received, exiting gracefully..." << std::endl;
    }
}

int main() {
    // Set up signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Initialize logging system
    Logger::getInstance().initialize(
        LogLevel::DEBUG,           // Log level
        LogTarget::BOTH,          // Output to both console and file
        "logs/inference_service.log", // Log file path
        10,                       // Max file size: 10MB
        5                         // Keep 5 backup files
    );
    
    ModuleLogger app_logger("APPLICATION");
    app_logger.info("=== Inference Service Starting ===");
    app_logger.info("Logging system initialized");
    
    std::cout << "Inference service with camera starting..." << std::endl;
    
    InferenceService service;
    
    // Initialize service
    app_logger.info("Initializing inference service");
    if (!service.initialize()) {
        app_logger.critical("Failed to initialize inference service");
        std::cerr << "Failed to initialize inference service" << std::endl;
        return -1;
    }
    
    app_logger.info("Inference service initialized successfully");
    std::cout << "Inference service initialized" << std::endl;
    
    // Start Web API server
    app_logger.info("Starting Web API server");
    if (service.startWebApi(8080)) {
        app_logger.info("Web API server started on http://localhost:8080");
        std::cout << "Web API server started on http://localhost:8080" << std::endl;
        std::cout << "API endpoints available for debugging and monitoring" << std::endl;
    } else {
        app_logger.warn("Failed to start Web API server, continuing without it");
        std::cout << "Warning: Web API server failed to start" << std::endl;
    }
    
    // Start camera
    app_logger.info("Starting camera subsystem");
    if (!service.startCamera(0)) {
        app_logger.critical("Failed to start camera - terminating application");
        std::cerr << "Failed to start camera" << std::endl;
        return -1;
    }
    
    app_logger.info("Camera subsystem started - entering main processing loop");
    std::cout << "Camera started. Press ESC in camera window to exit..." << std::endl;
    
    // Process camera frames
    while (service.isCameraRunning() && !shutdown_requested) {
        if (!service.processFrame()) {
            break; // Exit on ESC key or error
        }
        
        // Small delay to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Check if shutdown was requested via signal
    if (shutdown_requested) {
        app_logger.info("Shutdown requested via signal");
    }
    
    // Cleanup
    app_logger.info("Application shutdown initiated");
    
    // Stop camera first
    service.stopCamera();
    
    // Stop Web API server
    service.stopWebApi();
    
    // Stop inference service
    service.stop();
    
    app_logger.info("=== Inference Service Shutdown Complete ===");
    
    // Flush and shutdown logging system gracefully
    try {
        Logger::getInstance().flush();
        Logger::getInstance().shutdown();
    } catch (const std::exception& e) {
        std::cerr << "Warning: Exception during logger shutdown: " << e.what() << std::endl;
    }
    
    std::cout << "Inference service stopped gracefully" << std::endl;
    return 0;
}