#!/bin/bash

# Build Test Script
# 用于编译各种测试文件的便利脚本

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔨 Building Tests${NC}"
echo "=================="

# Project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TESTS_DIR="$PROJECT_ROOT/tests"
BUILD_DIR="$PROJECT_ROOT/build"
INCLUDE_DIR="$PROJECT_ROOT/include"
SRC_DIR="$PROJECT_ROOT/src"

# Create test build directory
TEST_BUILD_DIR="$BUILD_DIR/tests"
mkdir -p "$TEST_BUILD_DIR"

# Common compile flags
COMMON_FLAGS="-std=c++17 -I$INCLUDE_DIR -I$PROJECT_ROOT/vcpkg/installed/x64-windows/include"
COMMON_LIBS="-L$PROJECT_ROOT/vcpkg/installed/x64-windows/lib"

# OpenCV libraries (adjust based on your installation)
OPENCV_LIBS="-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio"

echo -e "${YELLOW}Compiling unit tests...${NC}"

# Build unit tests (now header-only, no separate .cpp files needed)
if [ -f "$TESTS_DIR/unit/test_logger.cpp" ]; then
    echo "Building test_logger..."
    g++ $COMMON_FLAGS "$TESTS_DIR/unit/test_logger.cpp" \
        -o "$TEST_BUILD_DIR/test_logger" || echo -e "${RED}Failed to build test_logger${NC}"
fi

echo -e "${YELLOW}Compiling performance tests...${NC}"

# Build performance tests
if [ -f "$TESTS_DIR/performance/perf_frame_processing.cpp" ]; then
    echo "Building perf_frame_processing..."
    g++ $COMMON_FLAGS "$TESTS_DIR/performance/perf_frame_processing.cpp" \
        $COMMON_LIBS $OPENCV_LIBS \
        -o "$TEST_BUILD_DIR/perf_frame_processing" || echo -e "${RED}Failed to build perf_frame_processing${NC}"
fi

echo -e "${YELLOW}Compiling temporary tests...${NC}"

# Build temp tests
if [ -f "$TESTS_DIR/temp/temp_quick_test.cpp" ]; then
    echo "Building temp_quick_test..."
    g++ $COMMON_FLAGS "$TESTS_DIR/temp/temp_quick_test.cpp" \
        $COMMON_LIBS $OPENCV_LIBS \
        -o "$TEST_BUILD_DIR/temp_quick_test" || echo -e "${RED}Failed to build temp_quick_test${NC}"
fi

echo -e "${YELLOW}Compiling manual tests...${NC}"

# Build manual tests
if [ -f "$TESTS_DIR/manual/test_camera.cpp" ]; then
    echo "Building test_camera..."
    g++ $COMMON_FLAGS "$TESTS_DIR/manual/test_camera.cpp" \
        $COMMON_LIBS $OPENCV_LIBS \
        -o "$TEST_BUILD_DIR/test_camera" || echo -e "${RED}Failed to build test_camera${NC}"
fi

echo ""
echo -e "${GREEN}✅ Build process completed!${NC}"
echo ""
echo -e "${BLUE}Available test executables in $TEST_BUILD_DIR:${NC}"
ls -la "$TEST_BUILD_DIR" 2>/dev/null || echo "No executables found"

echo ""
echo -e "${BLUE}Usage examples:${NC}"
echo "  Run unit tests:        ./build/tests/test_logger"
echo "  Run performance test:  ./build/tests/perf_frame_processing"
echo "  Run quick test:        ./build/tests/temp_quick_test"
echo "  Run camera test:       ./build/tests/test_camera"