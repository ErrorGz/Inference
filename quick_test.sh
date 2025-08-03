#!/bin/bash

# Quick test script for testing graceful shutdown
# 快速测试脚本，用于测试优雅退出

echo "🧪 Starting quick test of graceful shutdown..."
echo "程序将在 5 秒后自动发送 SIGINT 信号测试优雅退出"
echo "或者您可以手动按 Ctrl+C 测试"
echo ""

# Start the program in background
cd build
./bin/Debug/InferenceService.exe &
PROGRAM_PID=$!

echo "程序已启动，PID: $PROGRAM_PID"
echo "等待 5 秒后发送退出信号..."

# Wait 5 seconds then send SIGINT
sleep 5

echo "发送 SIGINT 信号..."
kill -INT $PROGRAM_PID

# Wait for program to exit
echo "等待程序优雅退出..."
wait $PROGRAM_PID
EXIT_CODE=$?

echo ""
echo "程序退出码: $EXIT_CODE"
if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ 程序正常退出"
else
    echo "⚠️  程序异常退出"
fi