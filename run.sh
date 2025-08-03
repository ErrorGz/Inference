#!/bin/bash

# Interactive Build and Run Script for Inference Service
# 推理服务交互式构建和运行脚本

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Project paths
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
EXECUTABLE="$BUILD_DIR/bin/Debug/InferenceService.exe"

# Function to print colored messages
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_header() {
    echo -e "${CYAN}${BOLD}"
    echo "=================================================="
    echo "    推理服务 - 交互式构建和运行脚本"
    echo "    Inference Service - Interactive Build & Run"
    echo "=================================================="
    echo -e "${NC}"
}

# Function to ask yes/no question
ask_yes_no() {
    local question="$1"
    local default="${2:-n}"
    local prompt
    
    if [[ "$default" == "y" || "$default" == "Y" ]]; then
        prompt="[Y/n]"
    else
        prompt="[y/N]"
    fi
    
    while true; do
        echo -ne "${YELLOW}$question $prompt: ${NC}"
        read -r answer
        
        # Use default if no answer provided
        if [[ -z "$answer" ]]; then
            answer="$default"
        fi
        
        case "$answer" in
            [Yy]|[Yy][Ee][Ss])
                return 0
                ;;
            [Nn]|[Nn][Oo])
                return 1
                ;;
            *)
                print_warning "请回答 yes/y 或 no/n"
                ;;
        esac
    done
}

# Function to check if build is needed
check_build_needed() {
    if [[ ! -f "$EXECUTABLE" ]]; then
        print_info "可执行文件不存在，需要编译"
        return 0
    fi
    
    # Check if source files are newer than executable
    local exe_time=$(stat -c %Y "$EXECUTABLE" 2>/dev/null || stat -f %m "$EXECUTABLE" 2>/dev/null || echo 0)
    local newest_source=0
    
    # Check main.cpp
    if [[ -f "$PROJECT_ROOT/main.cpp" ]]; then
        local main_time=$(stat -c %Y "$PROJECT_ROOT/main.cpp" 2>/dev/null || stat -f %m "$PROJECT_ROOT/main.cpp" 2>/dev/null || echo 0)
        if [[ $main_time -gt $newest_source ]]; then
            newest_source=$main_time
        fi
    fi
    
    # Check header files
    for hpp_file in "$PROJECT_ROOT/include"/*.hpp; do
        if [[ -f "$hpp_file" ]]; then
            local hpp_time=$(stat -c %Y "$hpp_file" 2>/dev/null || stat -f %m "$hpp_file" 2>/dev/null || echo 0)
            if [[ $hpp_time -gt $newest_source ]]; then
                newest_source=$hpp_time
            fi
        fi
    done
    
    if [[ $newest_source -gt $exe_time ]]; then
        print_info "源文件已更新，建议重新编译"
        return 0
    fi
    
    return 1
}

# Function to build the project
build_project() {
    print_info "开始编译项目..."
    
    # Create build directory if it doesn't exist
    if [[ ! -d "$BUILD_DIR" ]]; then
        print_info "创建构建目录: $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_info "配置 CMake..."
    if ! cmake ..; then
        print_error "CMake 配置失败"
        return 1
    fi
    
    # Build the project
    print_info "编译项目..."
    if ! cmake --build .; then
        print_error "编译失败"
        return 1
    fi
    
    print_success "编译完成！"
    return 0
}

# Function to run the project
run_project() {
    if [[ ! -f "$EXECUTABLE" ]]; then
        print_error "可执行文件不存在: $EXECUTABLE"
        return 1
    fi
    
    print_info "启动推理服务..."
    print_info "程序路径: $EXECUTABLE"
    echo
    print_warning "提示："
    print_warning "  - 摄像头窗口中按 ESC 键退出程序"
    print_warning "  - 或者在终端中按 Ctrl+C 强制退出"
    echo
    
    # Change to build directory to run
    cd "$BUILD_DIR"
    
    # Run the executable
    if ! ./bin/Debug/InferenceService.exe; then
        local exit_code=$?
        print_error "程序异常退出，退出码: $exit_code"
        return $exit_code
    fi
    
    print_success "程序正常退出"
    return 0
}

# Function to show project status
show_status() {
    echo
    print_header
    
    print_info "项目状态检查："
    echo
    
    # Check project structure
    if [[ -f "$PROJECT_ROOT/main.cpp" ]]; then
        print_success "主程序文件: main.cpp ✓"
    else
        print_error "主程序文件: main.cpp ✗"
    fi
    
    # Check header files
    local hpp_count=$(find "$PROJECT_ROOT/include" -name "*.hpp" 2>/dev/null | wc -l)
    if [[ $hpp_count -gt 0 ]]; then
        print_success "头文件: $hpp_count 个 .hpp 文件 ✓"
    else
        print_warning "头文件: 未找到 .hpp 文件"
    fi
    
    # Check CMakeLists.txt
    if [[ -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
        print_success "构建配置: CMakeLists.txt ✓"
    else
        print_error "构建配置: CMakeLists.txt ✗"
    fi
    
    # Check build directory
    if [[ -d "$BUILD_DIR" ]]; then
        print_success "构建目录: $BUILD_DIR ✓"
    else
        print_warning "构建目录: $BUILD_DIR (不存在，将自动创建)"
    fi
    
    # Check executable
    if [[ -f "$EXECUTABLE" ]]; then
        local exe_size=$(stat -c %s "$EXECUTABLE" 2>/dev/null || stat -f %z "$EXECUTABLE" 2>/dev/null || echo 0)
        local exe_date=$(stat -c %y "$EXECUTABLE" 2>/dev/null || stat -f %Sm "$EXECUTABLE" 2>/dev/null || echo "未知")
        print_success "可执行文件: $(basename "$EXECUTABLE") ($(($exe_size / 1024)) KB, $exe_date) ✓"
    else
        print_warning "可执行文件: 不存在，需要编译"
    fi
    
    echo
}

# Function to show help
show_help() {
    echo
    echo -e "${BOLD}用法:${NC}"
    echo "  $0 [选项]"
    echo
    echo -e "${BOLD}选项:${NC}"
    echo "  -h, --help     显示此帮助信息"
    echo "  -s, --status   显示项目状态"
    echo "  -b, --build    仅编译，不运行"
    echo "  -r, --run      仅运行，不编译"
    echo "  -f, --force    强制重新编译"
    echo
    echo -e "${BOLD}交互模式:${NC}"
    echo "  不带参数运行脚本将进入交互模式"
    echo
    echo -e "${BOLD}示例:${NC}"
    echo "  $0              # 交互模式"
    echo "  $0 --build      # 仅编译"
    echo "  $0 --run        # 仅运行"
    echo "  $0 --force      # 强制重新编译并询问是否运行"
    echo
}

# Main script logic
main() {
    local build_only=false
    local run_only=false
    local force_build=false
    local show_status_only=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -s|--status)
                show_status_only=true
                shift
                ;;
            -b|--build)
                build_only=true
                shift
                ;;
            -r|--run)
                run_only=true
                shift
                ;;
            -f|--force)
                force_build=true
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Show status only
    if [[ "$show_status_only" == true ]]; then
        show_status
        exit 0
    fi
    
    # Show header
    print_header
    
    # Build only mode
    if [[ "$build_only" == true ]]; then
        if build_project; then
            print_success "编译完成！"
            exit 0
        else
            exit 1
        fi
    fi
    
    # Run only mode
    if [[ "$run_only" == true ]]; then
        if run_project; then
            exit 0
        else
            exit 1
        fi
    fi
    
    # Interactive mode
    local should_build=false
    local should_run=false
    
    # Determine if build is needed
    if [[ "$force_build" == true ]] || check_build_needed; then
        if [[ "$force_build" == true ]]; then
            should_build=true
            print_info "强制重新编译模式"
        else
            if ask_yes_no "是否需要编译项目？" "y"; then
                should_build=true
            fi
        fi
    else
        print_success "可执行文件是最新的"
        if ask_yes_no "是否仍要重新编译？" "n"; then
            should_build=true
        fi
    fi
    
    # Build if requested
    if [[ "$should_build" == true ]]; then
        if ! build_project; then
            print_error "编译失败，无法继续"
            exit 1
        fi
    fi
    
    # Ask about running
    if ask_yes_no "是否运行程序？" "y"; then
        should_run=true
    fi
    
    # Run if requested
    if [[ "$should_run" == true ]]; then
        if ! run_project; then
            exit 1
        fi
    else
        print_info "程序未运行"
    fi
    
    print_success "脚本执行完成！"
}

# Trap Ctrl+C and cleanup
trap 'echo -e "\n${YELLOW}用户中断操作${NC}"; exit 130' INT

# Run main function
main "$@"