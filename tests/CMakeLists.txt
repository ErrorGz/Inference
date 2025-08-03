# Tests CMakeLists.txt
# 测试目录的 CMake 配置文件

cmake_minimum_required(VERSION 3.16)

# 如果作为子项目包含，则使用父项目的设置
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# 包含主项目的头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找 OpenCV (继承主项目配置)
if(NOT OpenCV_FOUND)
    find_package(OpenCV REQUIRED)
endif()

# 源文件路径
set(MAIN_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# 单元测试（现在只需要 .cpp 文件，其他都是头文件实现）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_logger.cpp")
    add_executable(test_logger unit/test_logger.cpp)
    target_link_libraries(test_logger ${OpenCV_LIBS})
endif()

# 性能测试
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/performance/perf_frame_processing.cpp")
    add_executable(perf_frame_processing performance/perf_frame_processing.cpp)
    target_link_libraries(perf_frame_processing ${OpenCV_LIBS})
endif()

# 临时测试
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/temp/temp_quick_test.cpp")
    add_executable(temp_quick_test temp/temp_quick_test.cpp)
    target_link_libraries(temp_quick_test ${OpenCV_LIBS})
endif()

# 手动测试
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/manual/test_camera.cpp")
    add_executable(test_camera
        manual/test_camera.cpp
    )
    target_link_libraries(test_camera ${OpenCV_LIBS})
endif()

# 设置输出目录
set_target_properties(
    test_logger
    perf_frame_processing
    temp_quick_test
    test_camera
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# 添加测试目标（如果使用 CTest）
enable_testing()

if(TARGET test_logger)
    add_test(NAME LoggerUnitTest COMMAND test_logger)
endif()

if(TARGET perf_frame_processing)
    add_test(NAME FrameProcessingPerformance COMMAND perf_frame_processing)
endif()

if(TARGET temp_quick_test)
    add_test(NAME QuickTest COMMAND temp_quick_test)
endif()

# 自定义目标：运行所有测试
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_logger perf_frame_processing temp_quick_test
    COMMENT "Running all tests"
)