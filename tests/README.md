# 测试目录说明 (Tests Directory)

这个目录包含了推理服务项目的所有测试代码和临时实验代码。

## 目录结构

```
tests/
├── README.md           # 本说明文档
├── unit/              # 单元测试
├── integration/       # 集成测试
├── performance/       # 性能测试
├── manual/           # 手动测试脚本
└── temp/             # 临时测试代码和实验代码
```

## 各目录用途

### 📋 unit/ - 单元测试
- **用途**: 测试单个类或函数的功能
- **内容**: 
  - 推理服务类的单元测试
  - 性能监控类的单元测试
  - 日志系统的单元测试
- **命名规范**: `test_<module_name>.cpp`
- **示例**: `test_inference_service.cpp`, `test_logger.cpp`

### 🔗 integration/ - 集成测试
- **用途**: 测试多个模块之间的协作
- **内容**:
  - 摄像头与推理服务的集成测试
  - 日志系统与性能监控的集成测试
  - 完整工作流程测试
- **命名规范**: `integration_<feature_name>.cpp`
- **示例**: `integration_camera_inference.cpp`

### ⚡ performance/ - 性能测试
- **用途**: 测试系统性能和基准测试
- **内容**:
  - 帧处理性能测试
  - 内存使用测试
  - 并发性能测试
  - 压力测试
- **命名规范**: `perf_<test_name>.cpp`
- **示例**: `perf_frame_processing.cpp`, `perf_memory_usage.cpp`

### 🖱️ manual/ - 手动测试
- **用途**: 需要人工交互的测试脚本
- **内容**:
  - 摄像头功能验证
  - UI交互测试
  - 硬件兼容性测试
- **命名规范**: `manual_<test_name>.cpp`
- **示例**: `manual_camera_test.cpp`, `manual_ui_test.cpp`

### 🧪 temp/ - 临时测试代码
- **用途**: 临时实验、原型验证、调试代码
- **内容**:
  - 快速原型测试
  - 功能验证代码
  - 调试辅助工具
  - 实验性功能
- **命名规范**: `temp_<description>.cpp` 或 `experiment_<name>.cpp`
- **示例**: `temp_opencv_test.cpp`, `experiment_new_algorithm.cpp`

## 编译和运行

### 单独编译测试文件
```bash
# 编译单个测试文件
g++ -std=c++17 -I../include tests/unit/test_logger.cpp -o test_logger

# 或者使用 CMake（如果配置了测试目标）
cmake --build . --target test_logger
```

### 运行所有测试
```bash
# 运行单元测试
./run_unit_tests.sh

# 运行集成测试
./run_integration_tests.sh

# 运行性能测试
./run_performance_tests.sh
```

## 测试规范

### 代码规范
1. **包含必要的头文件**: 测试框架、被测试的模块
2. **使用断言**: 验证预期结果
3. **异常处理**: 测试异常情况和边界条件
4. **清理资源**: 测试后清理临时文件和资源

### 命名规范
- 测试文件: `test_<module>.cpp` 或 `<type>_<name>.cpp`
- 测试函数: `test_<function_name>()` 或 `<function_name>_test()`
- 测试类: `<Module>Test` 或 `Test<Module>`

### 文档规范
- 每个测试文件应包含简要说明
- 复杂测试应有详细注释
- 性能测试应记录基准数据

## 示例测试结构

```cpp
// tests/unit/test_logger.cpp
#include "logger.h"
#include <cassert>
#include <iostream>

void test_logger_initialization() {
    Logger& logger = Logger::getInstance();
    logger.initialize(LogLevel::DEBUG, LogTarget::CONSOLE, "test.log");
    
    // 测试日志级别设置
    assert(logger.getLogLevel() == LogLevel::DEBUG);
    
    std::cout << "✅ Logger initialization test passed" << std::endl;
}

void test_module_logger() {
    ModuleLogger test_logger("TEST");
    test_logger.info("Test message");
    
    std::cout << "✅ Module logger test passed" << std::endl;
}

int main() {
    test_logger_initialization();
    test_module_logger();
    
    std::cout << "🎉 All logger tests passed!" << std::endl;
    return 0;
}
```

## 注意事项

1. **临时文件管理**: `temp/` 目录中的文件可能会被定期清理
2. **版本控制**: 
   - 正式测试代码应提交到版本控制
   - `temp/` 目录中的实验代码可选择性提交
3. **资源清理**: 测试后清理生成的临时文件和日志
4. **性能影响**: 性能测试应在独立环境中运行，避免影响开发环境

## 工具和框架

推荐使用的测试工具：
- **Google Test (gtest)**: C++ 单元测试框架
- **Google Benchmark**: C++ 性能测试框架
- **Valgrind**: 内存泄漏检测
- **AddressSanitizer**: 内存错误检测

---

💡 **提示**: 这个测试目录是为了保持主项目代码的整洁，所有实验性和测试性代码都应放在这里。