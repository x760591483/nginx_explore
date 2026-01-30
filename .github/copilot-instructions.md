# Nginx源码探索项目 - AI助手指南

这是一个nginx 1.20.2源码学习项目，专注于理解nginx架构和开发自定义HTTP模块。

## 项目架构概览

### 核心目录结构
- `src/` - nginx原始源码，采用模块化设计：`core/`(核心)、`http/`(HTTP模块)、`event/`(事件处理)、`os/`(操作系统抽象层)
- `mycode/` - 自定义模块开发目录，包含完整的HTTP模块示例(`ngx_http_mytest_module.c`)
- `mybuild/` - 编译输出目录，包含nginx可执行文件(`sbin/nginx`)和运行时配置
- `notes/` - 学习笔记，包含HTTP处理11阶段流程分析和模块开发指南
- `auto/` - nginx构建系统脚本，支持模块动态添加

### 关键架构组件
- **Master-Worker模型**: nginx采用单master多worker的进程模型，通过共享内存和信号通信
- **事件驱动**: 基于epoll等机制的异步事件处理，支持高并发连接
- **HTTP处理流程**: 11个处理阶段从连接建立到响应生成的完整链路
- **模块化架构**: 通过`ngx_module_t`结构体和钩子函数实现功能扩展

## 开发工作流

### 模块开发流程
1. **创建模块**: 在`mycode/`目录创建`.c`文件和`config`文件，参照`ngx_http_mytest_module.c`的标准结构
2. **配置编译**: 使用`./configure --add-module=mycode`自动集成模块到nginx构建系统
3. **编译测试**: 运行`./build_and_test.sh`进行自动化构建、配置生成和测试环境启动
4. **功能验证**: 使用`./test_mytest_module.sh`测试HTTP端点和模块功能
5. **调试分析**: 检查`mybuild/logs/error.log`和`mybuild/logs/access.log`获取运行时信息

### 关键命令模式
```bash
# 一键编译和测试环境
./build_and_test.sh

# 模块功能验证
./test_mytest_module.sh

# 手动编译流程
./configure --add-module=mycode && make

# 配置验证和启动
./objs/nginx -t -c $(pwd)/test_nginx.conf
./objs/nginx -c $(pwd)/test_nginx.conf
```

## 项目特定约定

### 模块开发约定
- **模块源码**: 使用`ngx_http_[name]_module.c`命名，位于`mycode/`目录
- **配置文件**: 每个模块需要`config`文件定义编译规则：
  ```bash
  ngx_addon_name=ngx_http_mytest_module
  HTTP_MODULES="$HTTP_MODULES ngx_http_mytest_module"
  NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_mytest_module.c"
  ```
- **配置结构体**: 使用`ngx_http_[name]_loc_conf_t`命名模式，必须包含create和merge函数
- **模块注册**: 通过`ngx_module_t`结构体定义模块接口，包含8个回调钩子

### nginx配置模式
```nginx
location /mytest {
    mytest_enabled on;  # 模块专用指令
}

location /mytest_disabled {
    mytest_enabled off; # 测试禁用状态
}
```

### 模块结构模板
参考`mycode/ngx_http_mytest_module.c`的标准结构：
- `ngx_http_[name]_loc_conf_t` - 配置结构体
- `ngx_http_[name]_commands[]` - 指令数组定义
- `ngx_http_[name]_module_ctx` - 模块上下文钩子
- `ngx_http_[name]_handler()` - HTTP请求处理器

## 关键文件说明

### 构建和测试脚本
- `build_and_test.sh`: 一键编译和测试脚本，自动生成`test_nginx.conf`配置文件并启动nginx
- `test_mytest_module.sh`: 模块功能验证脚本，测试启用/禁用状态和访问计数
- `mycode/config`: nginx模块构建配置，定义模块名称和源文件路径

### 学习资源文档
- `notes/http请求处理流程.md`: nginx HTTP处理11阶段详解，从连接建立到响应生成
- `notes/mytest_module_build_guide.md`: 模块开发完整指南，包含编译、配置、测试流程
- `README.md`: 整体学习方法和步骤，推荐"自顶向下"+"按需深入"方式

### 运行时配置
- `mybuild/conf/nginx.conf`: 编译后的默认配置文件
- `test_nginx.conf`: 由`build_and_test.sh`自动生成的测试配置，包含模块测试端点
- `mybuild/logs/`: 运行时日志目录，包含`error.log`和`access.log`

## 调试和开发技巧

### 模块开发最佳实践
1. **从示例开始**: `mycode/ngx_http_mytest_module.c`是一个功能完整的HTTP模块，包含配置结构体、指令处理、请求处理器等完整实现
2. **使用自动化脚本**: `build_and_test.sh`确保编译环境正确配置，自动生成测试配置文件
3. **日志调试**: 查看`mybuild/logs/error.log`获取运行时错误，模块中使用`ngx_log_error()`记录调试信息
4. **配置管理**: 模块配置结构体必须包含create和merge函数，参考示例的实现模式

### 调试工作流程
1. **修改-编译**: 修改模块代码后运行`make`重新编译，或使用`./build_and_test.sh`完整重构建
2. **配置验证**: 使用`./objs/nginx -t -c $(pwd)/test_nginx.conf`验证配置语法
3. **运行时调试**: 检查`mybuild/logs/`目录下的日志文件，模块通过`printf()`和`ngx_log_error()`输出调试信息
4. **功能测试**: 使用`curl localhost:8080/mytest`测试HTTP端点，或运行`./test_mytest_module.sh`进行批量测试

### 常见集成模式
- **模块注册**: 通过`ngx_module_t`结构体定义模块接口，包含8个回调钩子函数
- **配置指令**: 实现`ngx_command_t`数组处理nginx配置指令，如`mytest_enabled on/off`
- **请求处理**: 在HTTP处理阶段挂载处理函数，通过`postconfiguration`钩子注册handler
- **内存管理**: 使用nginx内存池(`ngx_pcalloc`)分配内存，避免内存泄漏

## 源码分析重点

### nginx架构理解重点
专注于理解而不是逐行阅读。关键入口点：
- **启动流程**: `src/core/nginx.c`的main函数展示master-worker模型初始化
- **HTTP模块**: `src/http/ngx_http.c`负责HTTP模块系统初始化和配置解析
- **请求处理**: `src/http/ngx_http_request.c`包含11阶段HTTP请求处理核心逻辑
- **事件系统**: `src/event/`目录包含epoll等事件驱动机制实现

### HTTP处理11阶段流程
参考`notes/http请求处理流程.md`了解完整链路：
1. NGX_HTTP_POST_READ_PHASE - 读取请求内容
2. NGX_HTTP_REALIP_PHASE - 真实IP处理
3. NGX_HTTP_REWRITE_PHASE - rewrite规则处理
4. NGX_HTTP_ACCESS_PHASE - 访问权限检查
5. ... (共11个阶段)

### 模块开发核心概念
- **模块结构**: `ngx_module_t`定义模块接口，包含type、ctx、commands等8个字段
- **配置解析**: 通过`ngx_command_t`数组定义配置指令的解析方法
- **钩子系统**: HTTP模块通过8个钩子函数在不同阶段介入处理流程
- **内存池**: nginx使用分层内存池管理内存，request、connection等都有独立内存池