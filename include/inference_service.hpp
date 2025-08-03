#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "performance_monitor.hpp"
#include "logger.hpp"
#include "web_api_server.hpp"

/**
 * @brief Inference Service Class - Header-only implementation
 * 
 * Handles model inference requests and camera processing
 */
class InferenceService {
public:
    InferenceService() : pImpl(std::make_unique<Impl>()) {}
    ~InferenceService() = default;

    /**
     * @brief Initialize inference service
     */
    bool initialize() {
        return pImpl->initialize();
    }

    /**
     * @brief Run inference service
     */
    void run() {
        pImpl->run();
    }

    /**
     * @brief Stop inference service
     */
    void stop() {
        pImpl->stop();
    }

    /**
     * @brief Execute inference
     */
    std::string inference(const std::string& input) {
        return pImpl->inference(input);
    }

    /**
     * @brief Start camera capture
     */
    bool startCamera(int camera_id = 0) {
        return pImpl->startCamera(camera_id);
    }

    /**
     * @brief Stop camera capture
     */
    void stopCamera() {
        pImpl->stopCamera();
    }

    /**
     * @brief Process camera frame
     */
    bool processFrame() {
        return pImpl->processFrame();
    }

    /**
     * @brief Check if camera is running
     */
    bool isCameraRunning() const {
        return pImpl->isCameraRunning();
    }

    /**
     * @brief Get performance monitor
     */
    const PerformanceMonitor& getPerformanceMonitor() const {
        return pImpl->performance_monitor;
    }

    /**
     * @brief Reset performance statistics
     */
    void resetPerformanceStats() {
        pImpl->performance_monitor.reset();
    }

    /**
     * @brief Start Web API server
     */
    bool startWebApi(int port = 8080) {
        return pImpl->startWebApi(port);
    }

    /**
     * @brief Stop Web API server
     */
    void stopWebApi() {
        pImpl->stopWebApi();
    }

    /**
     * @brief Check if Web API server is running
     */
    bool isWebApiRunning() const {
        return pImpl->isWebApiRunning();
    }

private:
    class Impl {
    public:
        bool running = false;
        bool camera_running = false;
        cv::VideoCapture camera;
        cv::Mat current_frame;
        PerformanceMonitor performance_monitor;
        
        // Web API server
        std::unique_ptr<WebApiServer> web_api_server;
        
        // Module loggers for different components
        ModuleLogger main_logger{"INFERENCE"};
        ModuleLogger camera_logger{"CAMERA"};
        ModuleLogger perf_logger{"PERFORMANCE"};
        
        bool initialize() {
            PERF_LOG_START("INFERENCE", initialization);
            main_logger.info("Starting inference engine initialization");
            
            try {
                // TODO: Add model loading and initialization logic here
                main_logger.debug("Loading inference models...");
                
                // Simulate initialization time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                main_logger.info("Inference engine initialized successfully");
                PERF_LOG_END("INFERENCE", initialization);
                return true;
            } catch (const std::exception& e) {
                main_logger.error("Failed to initialize inference engine: " + std::string(e.what()));
                return false;
            }
        }
        
        void run() {
            main_logger.info("Starting inference service main loop");
            running = true;
            
            // Simple service loop
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // TODO: Add service processing logic here
                main_logger.trace("Service heartbeat - main loop iteration");
            }
            
            main_logger.info("Inference service main loop stopped");
        }
        
        void stop() {
            main_logger.info("Stopping inference service");
            running = false;
            main_logger.info("Inference service stopped successfully");
        }
        
        std::string inference(const std::string& input) {
            // TODO: Add actual inference logic here
            return "Inference result: " + input;
        }
        
        bool startCamera(int camera_id = 0) {
            if (camera_running) {
                camera_logger.warn("Camera is already running, ignoring start request");
                return true;
            }
            
            PERF_LOG_START("CAMERA", startup);
            camera_logger.info("Starting camera with device ID: " + std::to_string(camera_id));
            
            try {
                // Open camera
                camera.open(camera_id);
                if (!camera.isOpened()) {
                    camera_logger.error("Failed to open camera device " + std::to_string(camera_id));
                    return false;
                }
                
                camera_logger.debug("Camera device opened successfully");
                
                // Set camera properties
                camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
                camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
                camera.set(cv::CAP_PROP_FPS, 30);
                
                // Log actual camera properties
                double actual_width = camera.get(cv::CAP_PROP_FRAME_WIDTH);
                double actual_height = camera.get(cv::CAP_PROP_FRAME_HEIGHT);
                double actual_fps = camera.get(cv::CAP_PROP_FPS);
                
                camera_logger.info("Camera properties set - Resolution: " + 
                                 std::to_string((int)actual_width) + "x" + std::to_string((int)actual_height) +
                                 ", FPS: " + std::to_string(actual_fps));
                
                camera_running = true;
                camera_logger.info("Camera started successfully");
                PERF_LOG_END("CAMERA", startup);
                return true;
                
            } catch (const std::exception& e) {
                camera_logger.error("Exception during camera startup: " + std::string(e.what()));
                return false;
            }
        }
        
        void stopCamera() {
            if (!camera_running) {
                camera_logger.debug("Camera stop requested but camera is not running");
                return;
            }
            
            camera_logger.info("Stopping camera");
            
            try {
                camera.release();
                camera_running = false;
                camera_logger.info("Camera stopped successfully");
            } catch (const std::exception& e) {
                camera_logger.error("Exception during camera shutdown: " + std::string(e.what()));
            }
        }
        
        bool processFrame() {
            if (!camera_running || !camera.isOpened()) {
                return false;
            }
            
            // Start frame timing
            performance_monitor.startFrame();
            
            // Capture frame
            camera >> current_frame;
            if (current_frame.empty()) {
                std::cerr << "Failed to capture frame" << std::endl;
                performance_monitor.endFrame(); // Still count failed frames
                return false;
            }
            
            // TODO: Add inference processing on the frame here
            // Simulate some processing time for demonstration
            // In real implementation, this would be your AI inference
            
            // Display frame (optional - for testing)
            cv::imshow("Camera Feed", current_frame);
            
            // End frame timing
            performance_monitor.endFrame();
            
            // Display performance stats periodically
            if (performance_monitor.shouldDisplayStats(5.0)) { // Every 5 seconds
                displayPerformanceStats();
            }
            
            // Process ESC key to exit
            int key = cv::waitKey(1) & 0xFF;
            if (key == 27) { // ESC key
                // Display final stats before exit
                std::cout << "\n" << performance_monitor.getPerformanceStats() << std::endl;
                return false;
            }
            
            return true;
        }
        
        void displayPerformanceStats() {
            // Log to both console and file
            std::stringstream stats;
            stats << std::fixed << std::setprecision(2);
            stats << "FPS: " << std::setprecision(1) << performance_monitor.getFPS();
            stats << ", Frame Time: " << std::setprecision(2) << performance_monitor.getCurrentFrameTime() << "ms";
            stats << " (avg: " << performance_monitor.getAverageFrameTime() << "ms)";
            stats << ", Total: " << performance_monitor.getTotalFrames();
            stats << ", Range: " << performance_monitor.getMinFrameTime() << "-" << performance_monitor.getMaxFrameTime() << "ms";
            
            perf_logger.info(stats.str());
            
            // Console output for immediate visibility
            std::cout << "\n=== Real-time Performance ===" << std::endl;
            std::cout << "FPS: " << std::fixed << std::setprecision(1) 
                      << performance_monitor.getFPS() << std::endl;
            std::cout << "Frame Time: " << std::setprecision(2) 
                      << performance_monitor.getCurrentFrameTime() << "ms (avg: " 
                      << performance_monitor.getAverageFrameTime() << "ms)" << std::endl;
            std::cout << "Total Frames: " << performance_monitor.getTotalFrames() << std::endl;
            std::cout << "Min/Max Frame Time: " << performance_monitor.getMinFrameTime() 
                      << "ms / " << performance_monitor.getMaxFrameTime() << "ms" << std::endl;
            std::cout << "================================\n" << std::endl;
        }
        
        bool isCameraRunning() const {
            return camera_running;
        }
        
        bool startWebApi(int port = 8080) {
            if (web_api_server && web_api_server->isRunning()) {
                main_logger.warn("Web API server is already running");
                return true;
            }
            
            try {
                main_logger.info("Starting Web API server on port " + std::to_string(port));
                
                web_api_server = std::make_unique<WebApiServer>(port);
                
                // Set references for API endpoints
                web_api_server->setPerformanceMonitor(&performance_monitor);
                web_api_server->setInferenceService(this);
                
                // Add custom routes
                addCustomRoutes();
                
                if (web_api_server->start()) {
                    main_logger.info("Web API server started successfully");
                    return true;
                } else {
                    main_logger.error("Failed to start Web API server");
                    web_api_server.reset();
                    return false;
                }
            } catch (const std::exception& e) {
                main_logger.error("Exception starting Web API server: " + std::string(e.what()));
                web_api_server.reset();
                return false;
            }
        }
        
        void stopWebApi() {
            if (web_api_server) {
                main_logger.info("Stopping Web API server");
                web_api_server->stop();
                web_api_server.reset();
                main_logger.info("Web API server stopped");
            }
        }
        
        bool isWebApiRunning() const {
            return web_api_server && web_api_server->isRunning();
        }
        
        void addCustomRoutes() {
            if (!web_api_server) return;
            
            // Camera control endpoints
            web_api_server->addRoute("/camera/start", [this](const std::string& method, const std::string& path, const std::string& body) {
                if (method == "POST") {
                    int camera_id = 0; // Default camera ID
                    
                    // Parse camera ID from body if provided
                    size_t pos = body.find("\"camera_id\":");
                    if (pos != std::string::npos) {
                        std::string id_str = body.substr(pos + 12);
                        size_t end = id_str.find_first_not_of("0123456789");
                        if (end != std::string::npos) {
                            id_str = id_str.substr(0, end);
                        }
                        try {
                            camera_id = std::stoi(id_str);
                        } catch (...) {
                            camera_id = 0;
                        }
                    }
                    
                    bool success = startCamera(camera_id);
                    std::ostringstream json;
                    json << "{";
                    json << "\"success\":" << (success ? "true" : "false") << ",";
                    json << "\"message\":\"" << (success ? "Camera started" : "Failed to start camera") << "\",";
                    json << "\"camera_id\":" << camera_id;
                    json << "}";
                    
                    return createJsonResponse(success ? 200 : 500, json.str());
                }
                return createJsonResponse(405, R"({"error":"Method not allowed"})");
            });
            
            web_api_server->addRoute("/camera/stop", [this](const std::string& method, const std::string& path, const std::string& body) {
                if (method == "POST") {
                    stopCamera();
                    return createJsonResponse(200, R"({"success":true,"message":"Camera stopped"})");
                }
                return createJsonResponse(405, R"({"error":"Method not allowed"})");
            });
            
            web_api_server->addRoute("/camera/status", [this](const std::string& method, const std::string& path, const std::string& body) {
                std::ostringstream json;
                json << "{";
                json << "\"running\":" << (camera_running ? "true" : "false") << ",";
                json << "\"status\":\"" << (camera_running ? "active" : "inactive") << "\"";
                if (camera_running && camera.isOpened()) {
                    json << ",\"properties\":{";
                    json << "\"width\":" << camera.get(cv::CAP_PROP_FRAME_WIDTH) << ",";
                    json << "\"height\":" << camera.get(cv::CAP_PROP_FRAME_HEIGHT) << ",";
                    json << "\"fps\":" << camera.get(cv::CAP_PROP_FPS);
                    json << "}";
                }
                json << "}";
                
                return createJsonResponse(200, json.str());
            });
            
            // Performance control endpoints
            web_api_server->addRoute("/performance/reset", [this](const std::string& method, const std::string& path, const std::string& body) {
                if (method == "POST") {
                    performance_monitor.reset();
                    return createJsonResponse(200, R"({"success":true,"message":"Performance statistics reset"})");
                }
                return createJsonResponse(405, R"({"error":"Method not allowed"})");
            });
            
            // Service control endpoints
            web_api_server->addRoute("/service/status", [this](const std::string& method, const std::string& path, const std::string& body) {
                std::ostringstream json;
                json << "{";
                json << "\"service_running\":" << (running ? "true" : "false") << ",";
                json << "\"camera_running\":" << (camera_running ? "true" : "false") << ",";
                json << "\"web_api_running\":" << (isWebApiRunning() ? "true" : "false") << ",";
                json << "\"total_frames\":" << performance_monitor.getTotalFrames() << ",";
                json << "\"current_fps\":" << std::fixed << std::setprecision(1) << performance_monitor.getFPS();
                json << "}";
                
                return createJsonResponse(200, json.str());
            });
        }
        
        std::string createJsonResponse(int status_code, const std::string& json_body) {
            std::string status_text;
            switch (status_code) {
                case 200: status_text = "OK"; break;
                case 400: status_text = "Bad Request"; break;
                case 404: status_text = "Not Found"; break;
                case 405: status_text = "Method Not Allowed"; break;
                case 500: status_text = "Internal Server Error"; break;
                default: status_text = "Unknown"; break;
            }
            
            std::ostringstream response;
            response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
            response << "Content-Type: application/json\r\n";
            response << "Content-Length: " << json_body.length() << "\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Connection: close\r\n";
            response << "\r\n";
            response << json_body;
            
            return response.str();
        }
    };

    std::unique_ptr<Impl> pImpl;
};