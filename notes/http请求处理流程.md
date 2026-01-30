# nginx HTTP请求处理流程详解

本文档详细分析了当客户端浏览器访问nginx时，nginx内部的完整处理流程，以及第三方HTTP模块（以MyTest模块为例）是如何被调用的。

## 概述

nginx的HTTP请求处理采用事件驱动和阶段化处理的方式，整个流程从客户端连接建立开始，经过11个处理阶段，最终生成响应返回给客户端。

## 完整处理流程

### 1. 连接建立阶段

**位置**: `src/http/ngx_http.c` → `ngx_http_init_connection()`

```c
// 当客户端连接建立时
ngx_http_init_connection(ngx_connection_t *c)
{
    // 设置连接的事件处理器
    rev->handler = ngx_http_wait_request_handler;
    // 等待客户端发送HTTP请求
}
```

**作用**: 初始化HTTP连接，设置事件处理器为等待请求状态。

### 2. 请求等待阶段

**位置**: `src/http/ngx_http_request.c` → `ngx_http_wait_request_handler()`

```c
static void ngx_http_wait_request_handler(ngx_event_t *rev)
{
    // 读取客户端发送的数据
    n = c->recv(c, b->last, size);
    
    // 创建HTTP请求对象
    c->data = ngx_http_create_request(c);
    
    // 切换到请求行处理
    rev->handler = ngx_http_process_request_line;
    ngx_http_process_request_line(rev);
}
```

**作用**: 等待并读取客户端发送的HTTP请求数据，创建请求对象。

### 3. 请求解析阶段

**位置**: `src/http/ngx_http_request.c` → `ngx_http_process_request_line()` → `ngx_http_process_request_headers()`

```c
// 解析HTTP请求行
static void ngx_http_process_request_line(ngx_event_t *rev)
{
    // 解析 GET /uri HTTP/1.1
    rc = ngx_http_parse_request_line(r, b);
    
    // 解析请求头
    rev->handler = ngx_http_process_request_headers;
    ngx_http_process_request_headers(rev);
}

// 解析HTTP请求头
static void ngx_http_process_request_headers(ngx_event_t *rev)
{
    // 解析所有HTTP头部
    rc = ngx_http_parse_header_line(r, b, 1);
    
    // 请求头解析完成后，开始处理请求
    ngx_http_process_request(r);
}
```

**作用**: 解析HTTP请求行和请求头，提取请求方法、URI、HTTP版本等信息。

### 4. 请求处理入口

**位置**: `src/http/ngx_http_request.c` → `ngx_http_process_request()`

```c
void ngx_http_process_request(ngx_http_request_t *r)
{
    // 设置事件处理器
    c->read->handler = ngx_http_request_handler;
    c->write->handler = ngx_http_request_handler;
    
    // 开始HTTP处理流程
    ngx_http_handler(r);
}
```

**作用**: 设置请求的事件处理器，启动HTTP处理框架。

### 5. HTTP处理框架

**位置**: `src/http/ngx_http_core_module.c` → `ngx_http_handler()`

```c
void ngx_http_handler(ngx_http_request_t *r)
{
    // 设置keepalive等连接属性
    r->keepalive = (r->http_version > NGX_HTTP_VERSION_10);
    
    // 设置阶段处理器为0，从第一个阶段开始
    r->phase_handler = 0;
    
    // 设置写事件处理器为阶段运行函数
    r->write_event_handler = ngx_http_core_run_phases;
    
    // 开始运行11个处理阶段
    ngx_http_core_run_phases(r);
}
```

**作用**: 初始化请求处理环境，启动11个处理阶段的执行。

### 6. 11个处理阶段执行

**位置**: `src/http/ngx_http_core_module.c` → `ngx_http_core_run_phases()`

```c
void ngx_http_core_run_phases(ngx_http_request_t *r)
{
    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);
    ph = cmcf->phase_engine.handlers;

    // 循环执行每个阶段的处理器
    while (ph[r->phase_handler].checker) {
        rc = ph[r->phase_handler].checker(r, &ph[r->phase_handler]);
        if (rc == NGX_OK) {
            return;
        }
    }
}
```

**作用**: 按顺序执行11个HTTP处理阶段，每个阶段都有特定的职责。

## nginx HTTP框架的11个处理阶段

### 阶段概览

1. **NGX_HTTP_POST_READ_PHASE (0)** - 读取请求后阶段
2. **NGX_HTTP_SERVER_REWRITE_PHASE (1)** - 服务器重写阶段
3. **NGX_HTTP_FIND_CONFIG_PHASE (2)** - 查找配置阶段
4. **NGX_HTTP_REWRITE_PHASE (3)** - 重写阶段
5. **NGX_HTTP_POST_REWRITE_PHASE (4)** - 重写后阶段
6. **NGX_HTTP_PREACCESS_PHASE (5)** - 访问前阶段
7. **NGX_HTTP_ACCESS_PHASE (6)** - 访问控制阶段
8. **NGX_HTTP_POST_ACCESS_PHASE (7)** - 访问后阶段
9. **NGX_HTTP_PRECONTENT_PHASE (8)** - 内容前阶段
10. **NGX_HTTP_CONTENT_PHASE (9)** - 内容生成阶段 ⭐
11. **NGX_HTTP_LOG_PHASE (10)** - 日志记录阶段

### 7. 第三方模块被调用的关键阶段

**位置**: `src/http/ngx_http_core_module.c` → `ngx_http_core_content_phase()`

```c
ngx_int_t ngx_http_core_content_phase(ngx_http_request_t *r, ngx_http_phase_handler_t *ph)
{
    printf("ngx_http_core_content_phase -> \n");
    
    // 检查是否有内容处理器
    if (r->content_handler) {
        r->write_event_handler = ngx_http_request_empty_handler;
        ngx_http_finalize_request(r, r->content_handler(r));
        return NGX_OK;
    }

    // 调用当前阶段的处理器 - 这里会调用你的模块
    rc = ph->handler(r);
    
    if (rc != NGX_DECLINED) {
        ngx_http_finalize_request(r, rc);
        return NGX_OK;
    }
    
    // 如果返回NGX_DECLINED，继续下一个处理器
    ph++;
    if (ph->checker) {
        r->phase_handler++;
        return NGX_AGAIN;
    }
}
```

**作用**: 在CONTENT_PHASE阶段调用所有注册的内容处理器，包括第三方模块。

### 8. 第三方模块处理函数被调用

**位置**: `mycode/ngx_http_mytest_module.c` → `ngx_http_mytest_handler()`

```c
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
    printf("ngx_http_mytest_handler --> \n");
    
    // 获取模块配置
    lcf = ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);
    
    // 检查模块是否启用
    if (!lcf->enabled) {
        return NGX_DECLINED;  // 拒绝处理，继续下一个处理器
    }
    
    // 生成响应内容
    // 设置响应头
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";
    
    // 发送响应头
    ngx_http_send_header(r);
    
    // 发送响应体
    ngx_http_output_filter(r, &out);
    
    return NGX_OK;  // 处理完成
}
```

**作用**: 第三方模块的核心处理逻辑，生成响应内容并发送给客户端。

### 9. 模块注册机制

**位置**: `mycode/ngx_http_mytest_module.c` → `ngx_http_mytest_init()`

```c
static ngx_int_t ngx_http_mytest_init(ngx_conf_t *cf)
{
    printf("ngx_http_mytest_init --> \n");
    
    // 获取HTTP核心模块的主配置
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    
    // 将处理函数注册到CONTENT_PHASE阶段
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    *h = ngx_http_mytest_handler;
    
    return NGX_OK;
}
```

**作用**: 在nginx启动时注册模块的处理函数到指定的处理阶段。

## 完整的调用链总结

```
1. 客户端连接 → ngx_http_init_connection()
2. 等待请求 → ngx_http_wait_request_handler()
3. 解析请求行 → ngx_http_process_request_line()
4. 解析请求头 → ngx_http_process_request_headers()
5. 开始处理 → ngx_http_process_request() → ngx_http_handler()
6. 运行阶段 → ngx_http_core_run_phases()
7. 内容阶段 → ngx_http_core_content_phase()
8. 你的模块 → ngx_http_mytest_handler()
9. 生成响应 → ngx_http_send_header() → ngx_http_output_filter()
10. 完成请求 → ngx_http_finalize_request()
```

## 关键要点

### 1. 阶段注册机制
- 第三方模块在nginx启动时通过`ngx_http_mytest_init()`注册到指定阶段
- 模块可以注册到多个阶段，但通常注册到`NGX_HTTP_CONTENT_PHASE`
- 注册顺序决定了模块的执行顺序

### 2. 处理顺序
- 在CONTENT_PHASE阶段，模块按照注册顺序被调用
- 每个模块都有机会处理请求
- 模块可以决定是否处理请求

### 3. 返回值含义
- **NGX_DECLINED**: 拒绝处理，继续下一个处理器
- **NGX_OK**: 处理完成，结束请求
- **NGX_ERROR**: 发生错误，结束请求
- **NGX_AGAIN**: 需要异步处理，等待事件

### 4. 配置获取
- 通过`ngx_http_get_module_loc_conf()`获取模块配置
- 配置在nginx启动时解析并合并
- 支持main、server、location三个级别的配置

### 5. 响应生成
- 通过`ngx_http_send_header()`发送响应头
- 通过`ngx_http_output_filter()`发送响应体
- 支持链式过滤器处理响应内容

## 实际应用示例

以MyTest模块为例，当客户端访问配置了该模块的location时：

1. **nginx启动时**: `ngx_http_mytest_init()`被调用，模块注册到CONTENT_PHASE
2. **客户端请求**: 经过前面的9个阶段，到达CONTENT_PHASE
3. **模块调用**: `ngx_http_mytest_handler()`被调用
4. **配置检查**: 检查`mytest_enabled`配置项
5. **内容生成**: 生成HTML响应内容
6. **响应发送**: 发送响应头和响应体
7. **请求完成**: 返回NGX_OK，请求处理完成

## 总结

nginx的HTTP请求处理流程是一个精心设计的阶段化处理系统，第三方模块通过注册到特定阶段来参与请求处理。理解这个流程对于开发nginx模块和调试nginx问题都非常重要。

第三方模块的开发要点：
1. 正确注册到合适的处理阶段
2. 合理处理返回值
3. 正确获取和使用配置
4. 遵循nginx的内存管理和事件处理机制 