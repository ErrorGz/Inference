/**
 * @file perf_frame_processing.cpp
 * @brief Performance test for frame processing pipeline
 */

#include "inference_service.hpp"
#include "performance_monitor.hpp"
#include "logger.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>

class FrameProcessingPerfTest {
public:
    static void test_synthetic_frame_processing() {
        std::cout << "Testing synthetic frame processing performance..." << std::endl;
        
        // Initialize logger for performance test
        Logger::getInstance().initialize(LogLevel::INFO, LogTarget::BOTH, 
                                       "test_logs/perf_test.log");
        
        ModuleLogger perf_logger("PERF_TEST");
        PerformanceMonitor monitor;
        
        // Create synthetic frames of different sizes
        std::vector<cv::Size> test_sizes = {
            {320, 240},   // QVGA
            {640, 480},   // VGA
            {1280, 720},  // HD
            {1920, 1080}  // Full HD
        };
        
        for (const auto& size : test_sizes) {
            perf_logger.info("Testing frame size: " + std::to_string(size.width) + 
                           "x" + std::to_string(size.height));
            
            test_frame_processing_at_resolution(size, monitor, perf_logger);
        }
        
        Logger::getInstance().shutdown();
    }
    
private:
    static void test_frame_processing_at_resolution(const cv::Size& size, 
                                                  PerformanceMonitor& monitor,
                                                  ModuleLogger& logger) {
        const int num_frames = 100;
        std::vector<double> processing_times;
        
        // Reset monitor for this test
        monitor.reset();
        
        for (int i = 0; i < num_frames; ++i) {
            monitor.startFrame();
            
            // Create synthetic frame
            cv::Mat frame = create_synthetic_frame(size);
            
            // Simulate typical image processing operations
            process_frame(frame);
            
            monitor.endFrame();
            processing_times.push_back(monitor.getCurrentFrameTime());
        }
        
        // Calculate statistics
        double avg_time = std::accumulate(processing_times.begin(), 
                                        processing_times.end(), 0.0) / num_frames;
        double min_time = *std::min_element(processing_times.begin(), 
                                          processing_times.end());
        double max_time = *std::max_element(processing_times.begin(), 
                                          processing_times.end());
        
        // Calculate percentiles
        std::sort(processing_times.begin(), processing_times.end());
        double p95_time = processing_times[static_cast<size_t>(num_frames * 0.95)];
        double p99_time = processing_times[static_cast<size_t>(num_frames * 0.99)];
        
        // Log results
        logger.info("Resolution: " + std::to_string(size.width) + "x" + std::to_string(size.height));
        logger.info("Frames processed: " + std::to_string(num_frames));
        logger.info("Average time: " + std::to_string(avg_time) + "ms");
        logger.info("Min time: " + std::to_string(min_time) + "ms");
        logger.info("Max time: " + std::to_string(max_time) + "ms");
        logger.info("P95 time: " + std::to_string(p95_time) + "ms");
        logger.info("P99 time: " + std::to_string(p99_time) + "ms");
        logger.info("Theoretical FPS: " + std::to_string(1000.0 / avg_time));
        
        // Console output for immediate feedback
        std::cout << "  Resolution: " << size.width << "x" << size.height << std::endl;
        std::cout << "  Average: " << std::fixed << std::setprecision(2) << avg_time << "ms" << std::endl;
        std::cout << "  Range: " << min_time << " - " << max_time << "ms" << std::endl;
        std::cout << "  P95/P99: " << p95_time << "/" << p99_time << "ms" << std::endl;
        std::cout << "  Theoretical FPS: " << std::setprecision(1) << (1000.0 / avg_time) << std::endl;
        std::cout << std::endl;
    }
    
    static cv::Mat create_synthetic_frame(const cv::Size& size) {
        // Create a synthetic frame with some patterns
        cv::Mat frame(size, CV_8UC3);
        
        // Fill with gradient pattern
        for (int y = 0; y < size.height; ++y) {
            for (int x = 0; x < size.width; ++x) {
                frame.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    static_cast<uchar>((x * 255) / size.width),
                    static_cast<uchar>((y * 255) / size.height),
                    static_cast<uchar>(((x + y) * 255) / (size.width + size.height))
                );
            }
        }
        
        return frame;
    }
    
    static void process_frame(cv::Mat& frame) {
        // Simulate typical image processing pipeline
        cv::Mat gray, blurred, edges;
        
        // Convert to grayscale
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        
        // Apply Gaussian blur
        cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);
        
        // Edge detection
        cv::Canny(blurred, edges, 50, 150);
        
        // Some morphological operations
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::Mat dilated;
        cv::dilate(edges, dilated, kernel);
        
        // Convert back to color (simulate output preparation)
        cv::Mat result;
        cv::cvtColor(dilated, result, cv::COLOR_GRAY2BGR);
        
        // Copy result back to original frame
        result.copyTo(frame);
    }
};

int main() {
    std::cout << "⚡ Frame Processing Performance Test" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
    
    try {
        FrameProcessingPerfTest::test_synthetic_frame_processing();
        
        std::cout << "🎉 Performance test completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Performance test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}