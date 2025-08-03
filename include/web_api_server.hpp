#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <map>
#include <functional>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include "logger.hpp"
#include "performance_monitor.hpp"

/**
 * @brief Simple HTTP Web API Server - Header-only implementation
 * 
 * Provides REST API endpoints for debugging and monitoring
 */
class WebApiServer {
public:
    using RequestHandler = std::function<std::string(const std::string& method, const std::string& path, const std::string& body)>;
    
    WebApiServer(int port = 8080) : port_(port), running_(false) {
        logger_ = std::make_unique<ModuleLogger>("WEBAPI");
        
#ifdef _WIN32
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            logger_->error("WSAStartup failed: " + std::to_string(result));
            throw std::runtime_error("Failed to initialize Winsock");
        }
#endif
        
        setupDefaultRoutes();
    }
    
    ~WebApiServer() {
        stop();
        
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    /**
     * @brief Start the web server
     */
    bool start() {
        if (running_) {
            logger_->warn("Server is already running");
            return true;
        }
        
        logger_->info("Starting Web API server on port " + std::to_string(port_));
        
        // Create socket
        server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket_ == INVALID_SOCKET) {
            logger_->error("Failed to create socket");
            return false;
        }
        
        // Set socket options
        int opt = 1;
        if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
            logger_->warn("Failed to set SO_REUSEADDR");
        }
        
        // Bind socket
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);
        
        if (bind(server_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            logger_->error("Failed to bind socket to port " + std::to_string(port_));
            closesocket(server_socket_);
            return false;
        }
        
        // Listen for connections
        if (listen(server_socket_, 10) == SOCKET_ERROR) {
            logger_->error("Failed to listen on socket");
            closesocket(server_socket_);
            return false;
        }
        
        running_ = true;
        server_thread_ = std::thread(&WebApiServer::serverLoop, this);
        
        logger_->info("Web API server started successfully on http://localhost:" + std::to_string(port_));
        logger_->info("Available endpoints:");
        for (const auto& route : routes_) {
            logger_->info("  " + route.first);
        }
        
        return true;
    }
    
    /**
     * @brief Stop the web server
     */
    void stop() {
        if (!running_) {
            return;
        }
        
        logger_->info("Stopping Web API server...");
        running_ = false;
        
        if (server_socket_ != INVALID_SOCKET) {
            closesocket(server_socket_);
            server_socket_ = INVALID_SOCKET;
        }
        
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        
        logger_->info("Web API server stopped");
    }
    
    /**
     * @brief Add a custom route handler
     */
    void addRoute(const std::string& path, RequestHandler handler) {
        routes_[path] = handler;
        logger_->debug("Added route: " + path);
    }
    
    /**
     * @brief Set performance monitor reference
     */
    void setPerformanceMonitor(const PerformanceMonitor* monitor) {
        performance_monitor_ = monitor;
    }
    
    /**
     * @brief Set inference service reference for status queries
     */
    void setInferenceService(const void* service) {
        inference_service_ = service;
    }
    
    /**
     * @brief Check if server is running
     */
    bool isRunning() const {
        return running_;
    }
    
    /**
     * @brief Get server port
     */
    int getPort() const {
        return port_;
    }

private:
    int port_;
    std::atomic<bool> running_;
    SOCKET server_socket_ = INVALID_SOCKET;
    std::thread server_thread_;
    std::unique_ptr<ModuleLogger> logger_;
    std::map<std::string, RequestHandler> routes_;
    
    // References to other components
    const PerformanceMonitor* performance_monitor_ = nullptr;
    const void* inference_service_ = nullptr;
    
    void setupDefaultRoutes() {
        // Health check endpoint
        addRoute("/health", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)method; (void)path; (void)body; // Suppress unused parameter warnings
            return createJsonResponse(200, R"({"status":"ok","message":"Web API server is running"})");
        });
        
        // Server status endpoint
        addRoute("/status", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)method; (void)path; (void)body;
            return handleStatusRequest();
        });
        
        // Performance metrics endpoint
        addRoute("/metrics", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)method; (void)path; (void)body;
            return handleMetricsRequest();
        });
        
        // Performance stats endpoint (detailed)
        addRoute("/stats", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)method; (void)path; (void)body;
            return handleStatsRequest();
        });
        
        // Logger control endpoint
        addRoute("/log-level", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)path;
            return handleLogLevelRequest(method, body);
        });
        
        // System info endpoint
        addRoute("/info", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)method; (void)path; (void)body;
            return handleInfoRequest();
        });
        
        // API documentation endpoint
        addRoute("/", [this](const std::string& method, const std::string& path, const std::string& body) {
            (void)method; (void)path; (void)body;
            return handleRootRequest();
        });
    }
    
    void serverLoop() {
        logger_->info("Server loop started");
        
        while (running_) {
            sockaddr_in client_addr{};
            int client_addr_len = sizeof(client_addr);
            
            SOCKET client_socket = accept(server_socket_, (sockaddr*)&client_addr, &client_addr_len);
            if (client_socket == INVALID_SOCKET) {
                if (running_) {
                    logger_->error("Failed to accept client connection");
                }
                continue;
            }
            
            // Handle client in separate thread for better concurrency
            std::thread client_thread(&WebApiServer::handleClient, this, client_socket);
            client_thread.detach();
        }
        
        logger_->info("Server loop ended");
    }
    
    void handleClient(SOCKET client_socket) {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            closesocket(client_socket);
            return;
        }
        
        buffer[bytes_received] = '\0';
        std::string request(buffer);
        
        // Parse HTTP request
        auto [method, path, body] = parseHttpRequest(request);
        
        logger_->debug("Request: " + method + " " + path);
        
        // Find matching route
        std::string response;
        auto it = routes_.find(path);
        if (it != routes_.end()) {
            try {
                response = it->second(method, path, body);
            } catch (const std::exception& e) {
                response = createJsonResponse(500, R"({"error":"Internal server error","message":")" + std::string(e.what()) + R"("})");
            }
        } else {
            response = createJsonResponse(404, R"({"error":"Not found","message":"Endpoint not found"})");
        }
        
        // Send response
        send(client_socket, response.c_str(), response.length(), 0);
        closesocket(client_socket);
    }
    
    std::tuple<std::string, std::string, std::string> parseHttpRequest(const std::string& request) {
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        // Extract body (after empty line)
        std::string body;
        std::string line;
        bool body_started = false;
        while (std::getline(iss, line)) {
            if (body_started) {
                body += line + "\n";
            } else if (line.empty() || line == "\r") {
                body_started = true;
            }
        }
        
        return {method, path, body};
    }
    
    std::string createJsonResponse(int status_code, const std::string& json_body) {
        std::string status_text;
        switch (status_code) {
            case 200: status_text = "OK"; break;
            case 400: status_text = "Bad Request"; break;
            case 404: status_text = "Not Found"; break;
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
    
    std::string handleStatusRequest() {
        std::ostringstream json;
        json << "{";
        json << "\"server\":{";
        json << "\"status\":\"running\",";
        json << "\"port\":" << port_ << ",";
        json << "\"uptime\":\"" << getCurrentTimestamp() << "\"";
        json << "},";
        json << "\"inference_service\":{";
        json << "\"status\":\"" << (inference_service_ ? "connected" : "disconnected") << "\"";
        json << "},";
        json << "\"performance_monitor\":{";
        json << "\"status\":\"" << (performance_monitor_ ? "connected" : "disconnected") << "\"";
        json << "}";
        json << "}";
        
        return createJsonResponse(200, json.str());
    }
    
    std::string handleMetricsRequest() {
        if (!performance_monitor_) {
            return createJsonResponse(503, R"({"error":"Performance monitor not available"})");
        }
        
        std::ostringstream json;
        json << std::fixed << std::setprecision(2);
        json << "{";
        json << "\"fps\":" << performance_monitor_->getFPS() << ",";
        json << "\"frame_time\":{";
        json << "\"current\":" << performance_monitor_->getCurrentFrameTime() << ",";
        json << "\"average\":" << performance_monitor_->getAverageFrameTime() << ",";
        json << "\"min\":" << performance_monitor_->getMinFrameTime() << ",";
        json << "\"max\":" << performance_monitor_->getMaxFrameTime();
        json << "},";
        json << "\"total_frames\":" << performance_monitor_->getTotalFrames() << ",";
        json << "\"timestamp\":\"" << getCurrentTimestamp() << "\"";
        json << "}";
        
        return createJsonResponse(200, json.str());
    }
    
    std::string handleStatsRequest() {
        if (!performance_monitor_) {
            return createJsonResponse(503, R"({"error":"Performance monitor not available"})");
        }
        
        std::string stats = performance_monitor_->getPerformanceStats();
        
        // Convert plain text stats to JSON format
        std::ostringstream json;
        json << "{";
        json << "\"detailed_stats\":\"" << escapeJsonString(stats) << "\",";
        json << "\"timestamp\":\"" << getCurrentTimestamp() << "\"";
        json << "}";
        
        return createJsonResponse(200, json.str());
    }
    
    std::string handleLogLevelRequest(const std::string& method, const std::string& body) {
        if (method == "GET") {
            // Get current log level
            LogLevel current_level = Logger::getInstance().getLogLevel();
            std::string level_str = logLevelToString(current_level);
            
            std::ostringstream json;
            json << "{";
            json << "\"current_level\":\"" << level_str << "\",";
            json << "\"available_levels\":[\"TRACE\",\"DEBUG\",\"INFO\",\"WARN\",\"ERROR\",\"CRITICAL\"]";
            json << "}";
            
            return createJsonResponse(200, json.str());
        } else if (method == "POST") {
            // Set new log level
            // Expected body: {"level": "DEBUG"}
            // Simple parsing (for demo purposes)
            std::string level_str;
            size_t pos = body.find("\"level\":");
            if (pos != std::string::npos) {
                size_t start = body.find("\"", pos + 8);
                size_t end = body.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    level_str = body.substr(start + 1, end - start - 1);
                }
            }
            
            LogLevel new_level = stringToLogLevel(level_str);
            Logger::getInstance().setLogLevel(new_level);
            
            logger_->info("Log level changed to: " + level_str);
            
            std::ostringstream json;
            json << "{";
            json << "\"message\":\"Log level changed to " << level_str << "\",";
            json << "\"new_level\":\"" << level_str << "\"";
            json << "}";
            
            return createJsonResponse(200, json.str());
        }
        
        return createJsonResponse(400, R"({"error":"Method not allowed"})");
    }
    
    std::string handleInfoRequest() {
        std::ostringstream json;
        json << "{";
        json << "\"application\":{";
        json << "\"name\":\"Inference Service\",";
        json << "\"version\":\"1.0.0\",";
        json << "\"build_time\":\"" << __DATE__ << " " << __TIME__ << "\"";
        json << "},";
        json << "\"system\":{";
        json << "\"timestamp\":\"" << getCurrentTimestamp() << "\",";
        json << "\"platform\":\"";
#ifdef _WIN32
        json << "Windows";
#elif __linux__
        json << "Linux";
#elif __APPLE__
        json << "macOS";
#else
        json << "Unknown";
#endif
        json << "\"";
        json << "},";
        json << "\"api\":{";
        json << "\"version\":\"1.0\",";
        json << "\"endpoints\":[";
        bool first = true;
        for (const auto& route : routes_) {
            if (!first) json << ",";
            json << "\"" << route.first << "\"";
            first = false;
        }
        json << "]";
        json << "}";
        json << "}";
        
        return createJsonResponse(200, json.str());
    }
    
    std::string handleRootRequest() {
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Inference Service API</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .endpoint { background: #f5f5f5; padding: 10px; margin: 10px 0; border-radius: 5px; }
        .method { color: #007acc; font-weight: bold; }
        .path { color: #d73a49; font-weight: bold; }
        code { background: #f6f8fa; padding: 2px 4px; border-radius: 3px; }
    </style>
</head>
<body>
    <h1>🚀 Inference Service Web API</h1>
    <p>Welcome to the Inference Service debugging API!</p>
    
    <h2>📋 Available Endpoints</h2>
    
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/health</span>
        <p>Health check endpoint</p>
        <code>curl http://localhost:)" + std::to_string(port_) + R"(/health</code>
    </div>
    
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/status</span>
        <p>Server and service status</p>
        <code>curl http://localhost:)" + std::to_string(port_) + R"(/status</code>
    </div>
    
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/metrics</span>
        <p>Real-time performance metrics</p>
        <code>curl http://localhost:)" + std::to_string(port_) + R"(/metrics</code>
    </div>
    
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/stats</span>
        <p>Detailed performance statistics</p>
        <code>curl http://localhost:)" + std::to_string(port_) + R"(/stats</code>
    </div>
    
    <div class="endpoint">
        <span class="method">GET/POST</span> <span class="path">/log-level</span>
        <p>Get or set logging level</p>
        <code>curl http://localhost:)" + std::to_string(port_) + R"(/log-level</code><br>
        <code>curl -X POST -d '{"level":"DEBUG"}' http://localhost:)" + std::to_string(port_) + R"(/log-level</code>
    </div>
    
    <div class="endpoint">
        <span class="method">GET</span> <span class="path">/info</span>
        <p>System and application information</p>
        <code>curl http://localhost:)" + std::to_string(port_) + R"(/info</code>
    </div>
    
    <h2>🛠️ Usage Examples</h2>
    <pre>
# Check if service is healthy
curl http://localhost:)" + std::to_string(port_) + R"(/health

# Get current performance metrics
curl http://localhost:)" + std::to_string(port_) + R"(/metrics

# Get detailed statistics
curl http://localhost:)" + std::to_string(port_) + R"(/stats

# Change log level to DEBUG
curl -X POST -H "Content-Type: application/json" \
     -d '{"level":"DEBUG"}' \
     http://localhost:)" + std::to_string(port_) + R"(/log-level

# Monitor metrics in real-time (every 2 seconds)
watch -n 2 "curl -s http://localhost:)" + std::to_string(port_) + R"(/metrics | jq ."
    </pre>
</body>
</html>
)";
        
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << html.length() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << html;
        
        return response.str();
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }
    
    std::string escapeJsonString(const std::string& str) {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    std::string logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case static_cast<LogLevel>(4): return "ERROR"; // Avoid ERROR macro conflict
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    LogLevel stringToLogLevel(const std::string& level_str) {
        if (level_str == "TRACE") return LogLevel::TRACE;
        if (level_str == "DEBUG") return LogLevel::DEBUG;
        if (level_str == "INFO") return LogLevel::INFO;
        if (level_str == "WARN") return LogLevel::WARN;
        if (level_str == "ERROR") return static_cast<LogLevel>(4); // Avoid ERROR macro conflict
        if (level_str == "CRITICAL") return LogLevel::CRITICAL;
        return LogLevel::INFO; // default
    }
};