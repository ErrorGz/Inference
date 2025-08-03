# 推理服务 (Inference Service)

这是一个基于 C++ 的推理服务项目。

## 项目结构

```
Inference/
├── CMakeLists.txt          # CMake 构建配置
├── main.cpp               # 主程序入口
├── include/               # 头文件目录
│   └── inference_service.h
├── src/                   # 源文件目录
│   └── inference_service.cpp
└── README.md             # 项目说明
```

## 构建说明

### 使用 CMake 构建

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 生成构建文件：
```bash
cmake ..
```

3. 编译项目：
```bash
cmake --build .
```

### Windows (Visual Studio)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make
```

## 运行

编译完成后，可执行文件位于 `build/bin/` 目录下：

```bash
./build/bin/InferenceService
```

## 开发说明

- 主要的推理逻辑在 `InferenceService` 类中实现
- 使用 PIMPL 设计模式来隐藏实现细节
- 支持跨平台编译（Windows/Linux/macOS）

## TODO

- [ ] 添加模型加载功能
- [ ] 实现网络服务接口
- [ ] 添加配置文件支持
- [ ] 集成推理框架（如 TensorRT、ONNX Runtime 等）
- [ ] 添加日志系统
- [ ] 添加单元测试