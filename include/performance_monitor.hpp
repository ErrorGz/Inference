#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <limits>

/**
 * @brief Performance Monitor Class - Header-only implementation
 * 
 * Tracks FPS, latency, and other performance metrics
 */
class PerformanceMonitor {
public:
    PerformanceMonitor() : pImpl(std::make_unique<Impl>()) {}
    ~PerformanceMonitor() = default;

    /**
     * @brief Start timing a frame
     */
    void startFrame() {
        pImpl->startFrame();
    }

    /**
     * @brief End timing a frame and update metrics
     */
    void endFrame() {
        pImpl->endFrame();
    }

    /**
     * @brief Get current FPS
     */
    double getFPS() const {
        return pImpl->getFPS();
    }

    /**
     * @brief Get average frame processing time
     */
    double getAverageFrameTime() const {
        return pImpl->getAverageFrameTime();
    }

    /**
     * @brief Get current frame processing time
     */
    double getCurrentFrameTime() const {
        return pImpl->getCurrentFrameTime();
    }

    /**
     * @brief Get minimum frame processing time
     */
    double getMinFrameTime() const {
        return pImpl->getMinFrameTime();
    }

    /**
     * @brief Get maximum frame processing time
     */
    double getMaxFrameTime() const {
        return pImpl->getMaxFrameTime();
    }

    /**
     * @brief Get total processed frames count
     */
    uint64_t getTotalFrames() const {
        return pImpl->getTotalFrames();
    }

    /**
     * @brief Get performance statistics as formatted string
     */
    std::string getPerformanceStats() const {
        return pImpl->getPerformanceStats();
    }

    /**
     * @brief Reset all performance counters
     */
    void reset() {
        pImpl->reset();
    }

    /**
     * @brief Check if should display stats (every N seconds)
     */
    bool shouldDisplayStats(double interval_seconds = 5.0) const {
        bool should_display = pImpl->shouldDisplayStats(interval_seconds);
        if (should_display) {
            pImpl->updateLastStatsTime();
        }
        return should_display;
    }

private:
    class Impl {
    public:
        using TimePoint = std::chrono::high_resolution_clock::time_point;
        using Duration = std::chrono::duration<double, std::milli>;

        // Timing variables
        TimePoint frame_start_time;
        TimePoint last_stats_time;
        TimePoint monitor_start_time;
        
        // Frame timing history (keep last N frames for smoothing)
        std::deque<double> frame_times;
        static constexpr size_t MAX_FRAME_HISTORY = 60; // Keep 60 frames for averaging
        
        // Statistics
        uint64_t total_frames = 0;
        double current_frame_time = 0.0;
        double min_frame_time = std::numeric_limits<double>::max();
        double max_frame_time = 0.0;
        
        // FPS calculation
        uint64_t fps_frame_count = 0;
        TimePoint fps_start_time;
        double current_fps = 0.0;
        
        Impl() {
            auto now = std::chrono::high_resolution_clock::now();
            monitor_start_time = now;
            last_stats_time = now;
            fps_start_time = now;
        }
        
        void startFrame() {
            frame_start_time = std::chrono::high_resolution_clock::now();
        }
        
        void endFrame() {
            auto frame_end_time = std::chrono::high_resolution_clock::now();
            
            // Calculate frame processing time
            Duration frame_duration = frame_end_time - frame_start_time;
            current_frame_time = frame_duration.count();
            
            // Update statistics
            total_frames++;
            fps_frame_count++;
            
            // Update min/max times
            min_frame_time = std::min(min_frame_time, current_frame_time);
            max_frame_time = std::max(max_frame_time, current_frame_time);
            
            // Add to frame time history
            frame_times.push_back(current_frame_time);
            if (frame_times.size() > MAX_FRAME_HISTORY) {
                frame_times.pop_front();
            }
            
            // Calculate FPS every second
            Duration fps_duration = frame_end_time - fps_start_time;
            if (fps_duration.count() >= 1000.0) { // 1 second
                current_fps = fps_frame_count / (fps_duration.count() / 1000.0);
                fps_frame_count = 0;
                fps_start_time = frame_end_time;
            }
        }
        
        double getFPS() const {
            return current_fps;
        }
        
        double getAverageFrameTime() const {
            if (frame_times.empty()) return 0.0;
            
            double sum = std::accumulate(frame_times.begin(), frame_times.end(), 0.0);
            return sum / frame_times.size();
        }
        
        double getCurrentFrameTime() const {
            return current_frame_time;
        }
        
        double getMinFrameTime() const {
            return (min_frame_time == std::numeric_limits<double>::max()) ? 0.0 : min_frame_time;
        }
        
        double getMaxFrameTime() const {
            return max_frame_time;
        }
        
        uint64_t getTotalFrames() const {
            return total_frames;
        }
        
        std::string getPerformanceStats() const {
            std::stringstream ss;
            auto now = std::chrono::high_resolution_clock::now();
            Duration total_duration = now - monitor_start_time;
            double total_seconds = total_duration.count() / 1000.0;
            
            ss << std::fixed << std::setprecision(2);
            ss << "=== Performance Statistics ===" << std::endl;
            ss << "Runtime: " << std::setprecision(1) << total_seconds << "s" << std::endl;
            ss << "Total Frames: " << total_frames << std::endl;
            ss << "Current FPS: " << std::setprecision(1) << current_fps << std::endl;
            ss << "Average FPS: " << std::setprecision(1) << (total_frames / total_seconds) << std::endl;
            ss << std::setprecision(2);
            ss << "Frame Time - Current: " << current_frame_time << "ms" << std::endl;
            ss << "Frame Time - Average: " << getAverageFrameTime() << "ms" << std::endl;
            ss << "Frame Time - Min: " << getMinFrameTime() << "ms" << std::endl;
            ss << "Frame Time - Max: " << max_frame_time << "ms" << std::endl;
            
            // Calculate frame time percentiles
            if (!frame_times.empty()) {
                std::vector<double> sorted_times(frame_times.begin(), frame_times.end());
                std::sort(sorted_times.begin(), sorted_times.end());
                
                size_t p95_idx = static_cast<size_t>(sorted_times.size() * 0.95);
                size_t p99_idx = static_cast<size_t>(sorted_times.size() * 0.99);
                
                ss << "Frame Time - P95: " << sorted_times[p95_idx] << "ms" << std::endl;
                ss << "Frame Time - P99: " << sorted_times[p99_idx] << "ms" << std::endl;
            }
            
            return ss.str();
        }
        
        void reset() {
            auto now = std::chrono::high_resolution_clock::now();
            monitor_start_time = now;
            last_stats_time = now;
            fps_start_time = now;
            
            frame_times.clear();
            total_frames = 0;
            fps_frame_count = 0;
            current_frame_time = 0.0;
            min_frame_time = std::numeric_limits<double>::max();
            max_frame_time = 0.0;
            current_fps = 0.0;
        }
        
        bool shouldDisplayStats(double interval_seconds) const {
            auto now = std::chrono::high_resolution_clock::now();
            Duration duration = now - last_stats_time;
            return duration.count() >= (interval_seconds * 1000.0);
        }
        
        void updateLastStatsTime() {
            last_stats_time = std::chrono::high_resolution_clock::now();
        }
    };

    std::unique_ptr<Impl> pImpl;
};