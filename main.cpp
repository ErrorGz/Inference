#include <iostream>
#include <thread>
#include <chrono>
#include "inference_service.h"

int main() {
    std::cout << "Inference service with camera starting..." << std::endl;
    
    InferenceService service;
    
    // Initialize service
    if (!service.initialize()) {
        std::cerr << "Failed to initialize inference service" << std::endl;
        return -1;
    }
    
    std::cout << "Inference service initialized" << std::endl;
    
    // Start camera
    if (!service.startCamera(0)) {
        std::cerr << "Failed to start camera" << std::endl;
        return -1;
    }
    
    std::cout << "Camera started. Press ESC in camera window to exit..." << std::endl;
    
    // Process camera frames
    while (service.isCameraRunning()) {
        if (!service.processFrame()) {
            break; // Exit on ESC key or error
        }
        
        // Small delay to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Cleanup
    service.stopCamera();
    service.stop();
    
    std::cout << "Inference service stopped" << std::endl;
    return 0;
}