# 🌐 推理服务 Web API 演示

推理服务现在提供了完整的 Web API 接口，支持通过 HTTP 请求进行远程监控和控制。

## 🚀 快速开始

### 1. 启动服务
```bash
# 启动推理服务（包含Web API）
./run.sh

# 或者直接运行
cd build && ./bin/Debug/InferenceService.exe
```

### 2. 验证 API 可用性
```bash
# 健康检查
curl http://localhost:8080/health

# 查看所有可用接口
curl http://localhost:8080/
```

## 📋 API 接口列表

### 🔍 **监控接口**

#### 健康检查
```bash
curl http://localhost:8080/health
```
**响应示例：**
```json
{
  "status": "ok",
  "message": "Web API server is running"
}
```

#### 服务器状态
```bash
curl http://localhost:8080/status
```
**响应示例：**
```json
{
  "server": {
    "status": "running",
    "port": 8080,
    "uptime": "2025-08-03T05:37:23Z"
  },
  "inference_service": {
    "status": "connected"
  },
  "performance_monitor": {
    "status": "connected"
  }
}
```

#### 性能指标
```bash
curl http://localhost:8080/metrics
```
**响应示例：**
```json
{
  "fps": 30.84,
  "frame_time": {
    "current": 4.73,
    "average": 9.35,
    "min": 2.71,
    "max": 876.35
  },
  "total_frames": 2156,
  "timestamp": "2025-08-03T05:38:15Z"
}
```

#### 详细统计
```bash
curl http://localhost:8080/stats
```
**响应示例：**
```json
{
  "detailed_stats": "=== Performance Statistics ===\\nRuntime: 125.3s\\nTotal Frames: 2156\\nCurrent FPS: 30.8\\nAverage FPS: 17.2\\n...",
  "timestamp": "2025-08-03T05:38:15Z"
}
```

### 🎮 **控制接口**

#### 服务状态
```bash
curl http://localhost:8080/service/status
```
**响应示例：**
```json
{
  "service_running": true,
  "camera_running": true,
  "web_api_running": true,
  "total_frames": 2156,
  "current_fps": 30.8
}
```

#### 摄像头状态
```bash
curl http://localhost:8080/camera/status
```
**响应示例：**
```json
{
  "running": true,
  "status": "active",
  "properties": {
    "width": 640,
    "height": 480,
    "fps": 30.0
  }
}
```

#### 启动摄像头
```bash
curl -X POST -H "Content-Type: application/json" \
     -d '{"camera_id": 0}' \
     http://localhost:8080/camera/start
```

#### 停止摄像头
```bash
curl -X POST http://localhost:8080/camera/stop
```

#### 重置性能统计
```bash
curl -X POST http://localhost:8080/performance/reset
```

### 📝 **日志控制**

#### 查看当前日志级别
```bash
curl http://localhost:8080/log-level
```
**响应示例：**
```json
{
  "current_level": "DEBUG",
  "available_levels": ["TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"]
}
```

#### 设置日志级别
```bash
# 设置为 INFO 级别
curl -X POST -H "Content-Type: application/json" \
     -d '{"level": "INFO"}' \
     http://localhost:8080/log-level

# 设置为 DEBUG 级别（更详细）
curl -X POST -H "Content-Type: application/json" \
     -d '{"level": "DEBUG"}' \
     http://localhost:8080/log-level
```

### 📊 **系统信息**
```bash
curl http://localhost:8080/info
```
**响应示例：**
```json
{
  "application": {
    "name": "Inference Service",
    "version": "1.0.0",
    "build_time": "Aug  3 2025 13:36:15"
  },
  "system": {
    "timestamp": "2025-08-03T05:38:15Z",
    "platform": "Windows"
  },
  "api": {
    "version": "1.0",
    "endpoints": ["/", "/camera/start", "/camera/status", ...]
  }
}
```

## 🛠️ **实用工具命令**

### 实时监控性能
```bash
# 每2秒刷新一次性能指标
watch -n 2 "curl -s http://localhost:8080/metrics | jq ."

# 或者使用我们的监控脚本
./test_api.sh monitor
```

### 批量测试所有接口
```bash
# 运行完整的API测试
./test_api.sh test
```

### 获取格式化的详细统计
```bash
# 使用 jq 格式化输出
curl -s http://localhost:8080/stats | jq -r '.detailed_stats'
```

### 监控日志变化
```bash
# 实时查看日志文件
tail -f build/logs/inference_service.log

# 过滤 API 请求日志
tail -f build/logs/inference_service.log | grep WEBAPI
```

## 🌐 **Web 界面**

访问 http://localhost:8080 可以看到一个简单的 Web 界面，包含：
- API 文档
- 使用示例
- 交互式测试

## 🔧 **高级用法**

### 1. 集成到监控系统
```bash
# Prometheus 风格的指标收集
curl -s http://localhost:8080/metrics | jq -r '
  "fps " + (.fps | tostring) + "\n" +
  "frame_time_avg " + (.frame_time.average | tostring) + "\n" +
  "total_frames " + (.total_frames | tostring)
'
```

### 2. 脚本化控制
```bash
#!/bin/bash
# 自动化测试脚本

# 重置计数器
curl -X POST http://localhost:8080/performance/reset

# 等待收集数据
sleep 10

# 获取结果
curl -s http://localhost:8080/metrics | jq '.fps'
```

### 3. 健康检查集成
```bash
# 适用于 Docker 健康检查
curl -f http://localhost:8080/health > /dev/null 2>&1 && echo "healthy" || echo "unhealthy"
```

## 🐛 **故障排除**

### API 不可访问
```bash
# 检查服务是否运行
ps aux | grep InferenceService

# 检查端口是否监听
netstat -an | grep :8080

# 检查防火墙设置
# Windows: 确保 8080 端口未被阻止
```

### JSON 格式化
```bash
# 如果没有 jq，可以使用 python
curl -s http://localhost:8080/metrics | python -m json.tool

# 或者安装 jq
# Windows: choco install jq
# Linux: sudo apt-get install jq
```

## 📈 **性能监控示例**

### 创建监控脚本
```bash
#!/bin/bash
# monitor.sh - 性能监控脚本

while true; do
    echo "=== $(date) ==="
    
    # 获取关键指标
    metrics=$(curl -s http://localhost:8080/metrics)
    fps=$(echo $metrics | jq -r '.fps')
    avg_time=$(echo $metrics | jq -r '.frame_time.average')
    total=$(echo $metrics | jq -r '.total_frames')
    
    echo "FPS: $fps"
    echo "Average Frame Time: ${avg_time}ms"
    echo "Total Frames: $total"
    echo ""
    
    sleep 5
done
```

### 性能告警
```bash
#!/bin/bash
# alert.sh - 性能告警脚本

fps=$(curl -s http://localhost:8080/metrics | jq -r '.fps')

if (( $(echo "$fps < 25" | bc -l) )); then
    echo "⚠️  WARNING: Low FPS detected: $fps"
    # 可以发送邮件、Slack通知等
fi
```

## 🎯 **最佳实践**

1. **监控频率**: 建议每5-10秒查询一次性能指标，避免过于频繁
2. **日志级别**: 生产环境建议使用 INFO 级别，调试时使用 DEBUG
3. **错误处理**: 所有 API 调用都应该包含错误处理
4. **超时设置**: 设置合理的 HTTP 超时时间（建议5-10秒）

---

🎉 **现在您可以通过 Web API 完全控制和监控推理服务了！**