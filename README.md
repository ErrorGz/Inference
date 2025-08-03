# 推理服务项目 (Inference Service)

一个基于 C++ 和 OpenCV 的实时推理服务项目，具有摄像头捕获、性能监控和工业级日志系统。

## 🚀 项目特性

### ✨ 核心功能
- **实时摄像头处理**: 支持多种摄像头设备，实时帧捕获和处理
- **性能监控**: 实时 FPS 监控、帧处理时间统计、性能指标分析
- **工业级日志**: 多级别日志、异步写入、自动轮转、模块化记录
- **优雅退出**: 支持信号处理，确保资源正确释放

### 🛠️ 技术架构
- **Header-Only 设计**: 所有核心组件采用 `.hpp` 头文件实现，便于集成
- **PIMPL 模式**: 隐藏实现细节，提供稳定的 API 接口
- **模块化设计**: 推理服务、性能监控、日志系统独立可复用
- **现代 C++**: 使用 C++17 标准，智能指针、RAII 等现代特性

## 📁 项目结构

```
Inference/
├── README.md                    # 项目说明文档
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 主程序入口
├── run.sh                      # 交互式运行脚本
├── quick_test.sh              # 快速测试脚本
│
├── include/                    # 头文件目录
│   ├── inference_service.hpp  # 推理服务 (Header-Only)
│   ├── logger.hpp             # 日志系统 (Header-Only)
│   └── performance_monitor.hpp # 性能监控 (Header-Only)
│
├── tests/                      # 测试目录
│   ├── README.md              # 测试说明
│   ├── CMakeLists.txt         # 测试构建配置
│   ├── build_test.sh          # 测试构建脚本
│   ├── unit/                  # 单元测试
│   ├── integration/           # 集成测试
│   ├── performance/           # 性能测试
│   ├── manual/               # 手动测试
│   └── temp/                 # 临时测试代码
│
└── build/                     # 构建输出目录
    ├── bin/                   # 可执行文件
    └── logs/                  # 日志文件
```

## 🔧 环境要求

### 系统要求
- **操作系统**: Windows 10/11, Linux, macOS
- **编译器**: 支持 C++17 的编译器
  - Windows: Visual Studio 2019+ 或 MinGW
  - Linux: GCC 7+ 或 Clang 5+
  - macOS: Xcode 10+

### 依赖库
- **CMake**: 3.16+
- **OpenCV**: 4.0+ (推荐 4.11.0)
- **vcpkg**: 包管理器 (可选，推荐)

### 硬件要求
- **摄像头**: USB 摄像头或内置摄像头
- **内存**: 最少 4GB RAM
- **存储**: 最少 1GB 可用空间

## 🚀 快速开始

### 1. 克隆项目
```bash
git clone https://github.com/ErrorGz/Inference.git
cd Inference
```

### 2. 安装依赖 (使用 vcpkg)
```bash
# 安装 OpenCV
vcpkg install opencv[contrib,cuda]:x64-windows  # Windows
vcpkg install opencv[contrib]:x64-linux         # Linux
```

### 3. 编译和运行

#### 使用交互式脚本 (推荐)
```bash
# 交互式模式
./run.sh

# 查看帮助
./run.sh --help

# 仅编译
./run.sh --build

# 仅运行
./run.sh --run

# 强制重新编译
./run.sh --force

# 查看项目状态
./run.sh --status
```

#### 使用 CMake (手动)
```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
cmake --build .

# 运行
./bin/Debug/InferenceService.exe    # Windows
./bin/InferenceService              # Linux/macOS
```

## 📖 使用说明

### 🎮 程序控制
- **启动程序**: 运行后自动初始化摄像头
- **查看摄像头**: 程序会显示实时摄像头画面
- **退出程序**: 
  - 在摄像头窗口按 `ESC` 键
  - 在终端按 `Ctrl+C`
  - 发送 SIGINT 或 SIGTERM 信号

### 📊 性能监控
程序会每 5 秒显示一次性能统计：
- **FPS**: 当前帧率
- **Frame Time**: 帧处理时间 (当前/平均)
- **Total Frames**: 总处理帧数
- **Min/Max Time**: 最小/最大处理时间

### 📝 日志系统
- **日志级别**: TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
- **输出目标**: 控制台 + 文件
- **日志文件**: `build/logs/inference_service.log`
- **自动轮转**: 文件大小超过 10MB 时自动轮转
- **备份保留**: 保留最近 5 个备份文件

## 🧪 测试

### 运行测试
```bash
# 构建所有测试
./tests/build_test.sh

# 或使用 CMake
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest --verbose
```

### 测试类型
- **单元测试**: 测试单个组件功能
- **集成测试**: 测试组件间协作
- **性能测试**: 基准测试和性能分析
- **手动测试**: 需要人工交互的功能测试

### 临时测试
在 `tests/temp/` 目录中可以放置临时测试代码：
```bash
# 编辑临时测试
vim tests/temp/temp_quick_test.cpp

# 快速编译运行
g++ -std=c++17 -I./include tests/temp/temp_quick_test.cpp -o temp_test && ./temp_test
```

## 🔧 开发指南

### 代码结构
```cpp
// 主要组件
InferenceService    // 推理服务主类
Logger             // 日志系统
PerformanceMonitor // 性能监控
ModuleLogger       // 模块化日志器
```

### 添加新功能
1. 在 `include/` 中创建 `.hpp` 头文件
2. 实现功能（Header-Only 设计）
3. 在 `main.cpp` 中集成
4. 添加相应测试

### 日志使用示例
```cpp
#include "logger.hpp"

// 初始化日志系统
Logger::getInstance().initialize(LogLevel::DEBUG, LogTarget::BOTH, "app.log");

// 使用模块日志器
ModuleLogger logger("MY_MODULE");
logger.info("This is an info message");
logger.error("This is an error message");

// 性能日志
PERF_LOG_START("MY_MODULE", operation_name);
// ... 执行操作 ...
PERF_LOG_END("MY_MODULE", operation_name);
```

### 性能监控示例
```cpp
#include "performance_monitor.hpp"

PerformanceMonitor monitor;

// 开始帧计时
monitor.startFrame();

// ... 处理帧 ...

// 结束帧计时
monitor.endFrame();

// 获取统计信息
double fps = monitor.getFPS();
double avg_time = monitor.getAverageFrameTime();
```

## 🐛 故障排除

### 常见问题

#### 1. 摄像头无法打开
```
错误: Failed to open camera device 0
解决: 
- 检查摄像头是否被其他程序占用
- 尝试不同的设备 ID (0, 1, 2...)
- 检查摄像头驱动是否正常
```

#### 2. OpenCV 库找不到
```
错误: Could not find OpenCV
解决:
- 确保 OpenCV 已正确安装
- 检查 CMAKE_TOOLCHAIN_FILE 路径
- 验证 vcpkg 集成是否正确
```

#### 3. 编译错误
```
错误: C++ standard not supported
解决:
- 确保编译器支持 C++17
- 检查 CMake 版本 >= 3.16
- 更新编译器到支持的版本
```

#### 4. 程序卡死退出
```
问题: 程序退出时卡死
解决:
- 使用 Ctrl+C 强制退出
- 检查日志系统是否正常关闭
- 确保摄像头资源正确释放
```

### 调试技巧
1. **启用详细日志**: 设置日志级别为 `LogLevel::TRACE`
2. **检查日志文件**: 查看 `build/logs/inference_service.log`
3. **使用调试器**: Visual Studio, GDB, LLDB
4. **性能分析**: 使用内置性能监控功能

## 📄 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 📞 联系方式

- **作者**: ErrorGz
- **邮箱**: errorgz@qq.com
- **项目地址**: https://github.com/ErrorGz/Inference

## 🎯 未来计划

- [ ] 添加 AI 模型推理功能
- [ ] 支持多摄像头同时处理
- [ ] 添加 Web API 接口
- [ ] GPU 加速优化
- [ ] Docker 容器化支持
- [ ] 更多图像处理算法

---

⭐ 如果这个项目对您有帮助，请给个 Star！