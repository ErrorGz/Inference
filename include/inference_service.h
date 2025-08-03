#pragma once

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

/**
 * @brief Inference Service Class
 * 
 * Handles model inference requests
 */
class InferenceService {
public:
    InferenceService();
    ~InferenceService();

    /**
     * @brief Initialize inference service
     * @return Returns true on success, false on failure
     */
    bool initialize();

    /**
     * @brief Run inference service
     */
    void run();

    /**
     * @brief Stop inference service
     */
    void stop();

    /**
     * @brief Execute inference
     * @param input Input data
     * @return Inference result
     */
    std::string inference(const std::string& input);

    /**
     * @brief Start camera capture
     * @param camera_id Camera device ID (default 0)
     * @return Returns true on success, false on failure
     */
    bool startCamera(int camera_id = 0);

    /**
     * @brief Stop camera capture
     */
    void stopCamera();

    /**
     * @brief Process camera frame
     * @return Returns true if frame was processed, false otherwise
     */
    bool processFrame();

    /**
     * @brief Check if camera is running
     * @return Returns true if camera is active
     */
    bool isCameraRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};