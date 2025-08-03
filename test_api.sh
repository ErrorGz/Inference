#!/bin/bash

# Web API Test Script
# 用于测试推理服务的 Web API 接口

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# API base URL
API_BASE="http://localhost:8080"

# Function to print colored messages
print_header() {
    echo -e "${CYAN}${BOLD}$1${NC}"
    echo -e "${CYAN}$(printf '=%.0s' {1..50})${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

# Function to test API endpoint
test_endpoint() {
    local method="$1"
    local endpoint="$2"
    local description="$3"
    local data="$4"
    
    echo -e "\n${BOLD}Testing: $description${NC}"
    echo -e "${YELLOW}$method $endpoint${NC}"
    
    if [[ "$method" == "GET" ]]; then
        response=$(curl -s -w "\n%{http_code}" "$API_BASE$endpoint" 2>/dev/null || echo -e "\nERROR")
    else
        response=$(curl -s -w "\n%{http_code}" -X "$method" -H "Content-Type: application/json" -d "$data" "$API_BASE$endpoint" 2>/dev/null || echo -e "\nERROR")
    fi
    
    if [[ "$response" == *"ERROR" ]]; then
        print_error "Failed to connect to API"
        return 1
    fi
    
    # Extract HTTP status code (last line)
    status_code=$(echo "$response" | tail -n1)
    # Extract response body (all but last line)
    body=$(echo "$response" | head -n -1)
    
    if [[ "$status_code" == "200" ]]; then
        print_success "HTTP $status_code - Success"
        
        # Pretty print JSON if possible
        if command -v jq >/dev/null 2>&1; then
            echo "$body" | jq . 2>/dev/null || echo "$body"
        else
            echo "$body"
        fi
    else
        print_error "HTTP $status_code - Error"
        echo "$body"
    fi
    
    return 0
}

# Function to check if service is running
check_service() {
    print_info "Checking if inference service is running..."
    
    if curl -s "$API_BASE/health" >/dev/null 2>&1; then
        print_success "Service is running on $API_BASE"
        return 0
    else
        print_error "Service is not running or not accessible"
        print_warning "Please make sure the inference service is started"
        print_info "Run: ./run.sh to start the service"
        return 1
    fi
}

# Main test function
run_tests() {
    print_header "🧪 Web API 接口测试"
    
    # Check service availability
    if ! check_service; then
        exit 1
    fi
    
    echo -e "\n${BOLD}📋 Available API Endpoints:${NC}"
    
    # Basic endpoints
    test_endpoint "GET" "/health" "健康检查"
    test_endpoint "GET" "/status" "服务器状态"
    test_endpoint "GET" "/info" "系统信息"
    
    # Performance monitoring
    test_endpoint "GET" "/metrics" "性能指标"
    test_endpoint "GET" "/stats" "详细统计"
    
    # Service control
    test_endpoint "GET" "/service/status" "服务状态"
    
    # Camera control
    test_endpoint "GET" "/camera/status" "摄像头状态"
    
    # Log level control
    test_endpoint "GET" "/log-level" "当前日志级别"
    
    # Performance reset (POST)
    test_endpoint "POST" "/performance/reset" "重置性能统计" ""
    
    # Log level change (POST)
    test_endpoint "POST" "/log-level" "设置日志级别为 INFO" '{"level":"INFO"}'
    
    echo -e "\n${BOLD}🎯 实用测试命令:${NC}"
    echo -e "${YELLOW}# 实时监控性能指标 (每2秒刷新)${NC}"
    echo "watch -n 2 \"curl -s $API_BASE/metrics | jq .\""
    echo ""
    echo -e "${YELLOW}# 获取详细性能统计${NC}"
    echo "curl -s $API_BASE/stats | jq -r '.detailed_stats'"
    echo ""
    echo -e "${YELLOW}# 检查所有服务状态${NC}"
    echo "curl -s $API_BASE/service/status | jq ."
    echo ""
    echo -e "${YELLOW}# 重置性能计数器${NC}"
    echo "curl -X POST $API_BASE/performance/reset"
    echo ""
    echo -e "${YELLOW}# 更改日志级别为 DEBUG${NC}"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{\"level\":\"DEBUG\"}' $API_BASE/log-level"
    
    print_header "✅ API 测试完成"
}

# Function to show real-time monitoring
monitor_metrics() {
    print_header "📊 实时性能监控"
    print_info "按 Ctrl+C 停止监控"
    echo ""
    
    while true; do
        clear
        echo -e "${CYAN}${BOLD}实时性能监控 - $(date)${NC}"
        echo -e "${CYAN}$(printf '=%.0s' {1..60})${NC}"
        
        # Get metrics
        metrics=$(curl -s "$API_BASE/metrics" 2>/dev/null || echo '{"error":"API不可用"}')
        
        if command -v jq >/dev/null 2>&1; then
            echo "$metrics" | jq .
        else
            echo "$metrics"
        fi
        
        echo -e "\n${YELLOW}按 Ctrl+C 停止监控${NC}"
        sleep 2
    done
}

# Parse command line arguments
case "${1:-test}" in
    "test")
        run_tests
        ;;
    "monitor")
        monitor_metrics
        ;;
    "help"|"-h"|"--help")
        echo "用法: $0 [test|monitor|help]"
        echo ""
        echo "命令:"
        echo "  test     运行 API 接口测试 (默认)"
        echo "  monitor  实时监控性能指标"
        echo "  help     显示此帮助信息"
        echo ""
        echo "示例:"
        echo "  $0 test      # 测试所有 API 接口"
        echo "  $0 monitor   # 实时监控性能"
        ;;
    *)
        print_error "未知命令: $1"
        echo "使用 '$0 help' 查看帮助"
        exit 1
        ;;
esac