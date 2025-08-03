#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "OpenCV Camera Test" << std::endl;
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;
    
    // Test camera availability
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera" << std::endl;
        std::cout << "Possible reasons:" << std::endl;
        std::cout << "1. No camera connected" << std::endl;
        std::cout << "2. Camera is being used by another application" << std::endl;
        std::cout << "3. Permission denied" << std::endl;
        return -1;
    }
    
    std::cout << "Camera opened successfully!" << std::endl;
    std::cout << "Camera properties:" << std::endl;
    std::cout << "Width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << std::endl;
    std::cout << "Height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
    std::cout << "FPS: " << cap.get(cv::CAP_PROP_FPS) << std::endl;
    
    // Test CUDA availability
    std::cout << "\nCUDA devices: " << cv::cuda::getCudaEnabledDeviceCount() << std::endl;
    if (cv::cuda::getCudaEnabledDeviceCount() > 0) {
        cv::cuda::DeviceInfo deviceInfo;
        std::cout << "CUDA device 0: " << deviceInfo.name() << std::endl;
    }
    
    cap.release();
    std::cout << "\nCamera test completed successfully!" << std::endl;
    return 0;
}