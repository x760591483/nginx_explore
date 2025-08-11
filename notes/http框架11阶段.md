# nginx HTTP框架的11个处理阶段

nginx的HTTP框架定义了11个处理阶段，这些阶段按顺序执行，每个阶段都有特定的职责。

## 阶段概览

### 1. NGX_HTTP_POST_READ_PHASE (0) - 读取请求后阶段
- **作用**：在读取完请求头后立即执行
- **用途**：进行请求的预处理，如设置变量、修改请求头等
- **典型模块**：`ngx_http_realip_module`（设置真实IP）

### 2. NGX_HTTP_SERVER_REWRITE_PHASE (1) - 服务器重写阶段
- **作用**：在server级别进行URL重写
- **用途**：执行server块中的rewrite指令
- **典型模块**：`ngx_http_rewrite_module`

### 3. NGX_HTTP_FIND_CONFIG_PHASE (2) - 查找配置阶段
- **作用**：根据请求的URI查找匹配的location配置
- **用途**：确定请求应该由哪个location块处理
- **内部阶段**：由nginx核心自动处理

### 4. NGX_HTTP_REWRITE_PHASE (3) - 重写阶段
- **作用**：在location级别进行URL重写
- **用途**：执行location块中的rewrite指令
- **典型模块**：`ngx_http_rewrite_module`

### 5. NGX_HTTP_POST_REWRITE_PHASE (4) - 重写后阶段
- **作用**：重写阶段完成后的处理
- **用途**：处理重写后的结果，可能需要重新查找配置
- **内部阶段**：由nginx核心自动处理

### 6. NGX_HTTP_PREACCESS_PHASE (5) - 访问前阶段
- **作用**：在访问控制之前的预处理
- **用途**：设置访问控制相关的变量或状态
- **典型模块**：`ngx_http_limit_req_module`、`ngx_http_limit_conn_module`

### 7. NGX_HTTP_ACCESS_PHASE (6) - 访问控制阶段
- **作用**：进行访问控制检查
- **用途**：验证客户端是否有权限访问资源
- **典型模块**：`ngx_http_access_module`、`ngx_http_auth_basic_module`

### 8. NGX_HTTP_POST_ACCESS_PHASE (7) - 访问后阶段
- **作用**：访问控制完成后的处理
- **用途**：处理访问控制的结果，如拒绝访问等
- **内部阶段**：由nginx核心自动处理

### 9. NGX_HTTP_PRECONTENT_PHASE (8) - 内容前阶段
- **作用**：在生成内容之前的预处理
- **用途**：设置内容处理相关的变量或状态
- **典型模块**：`ngx_http_try_files_module`

### 10. NGX_HTTP_CONTENT_PHASE (9) - 内容生成阶段
- **作用**：生成响应内容的核心阶段
- **用途**：处理请求并生成响应
- **典型模块**：`ngx_http_proxy_module`、`ngx_http_fastcgi_module`、`ngx_http_static_module`

### 11. NGX_HTTP_LOG_PHASE (10) - 日志记录阶段
- **作用**：记录请求日志
- **用途**：记录访问日志、错误日志等
- **典型模块**：`ngx_http_log_module`

## 源码中的阶段定义

```c
typedef enum {
    NGX_HTTP_POST_READ_PHASE = 0,
    NGX_HTTP_SERVER_REWRITE_PHASE,
    NGX_HTTP_FIND_CONFIG_PHASE,
    NGX_HTTP_REWRITE_PHASE,
    NGX_HTTP_POST_REWRITE_PHASE,
    NGX_HTTP_PREACCESS_PHASE,
    NGX_HTTP_ACCESS_PHASE,
    NGX_HTTP_POST_ACCESS_PHASE,
    NGX_HTTP_PRECONTENT_PHASE,
    NGX_HTTP_CONTENT_PHASE,
    NGX_HTTP_LOG_PHASE
} ngx_http_phases;
```

## 阶段执行流程

### 核心执行函数

```c
void ngx_http_core_run_phases(ngx_http_request_t *r)
{
    ngx_int_t                   rc;
    ngx_http_phase_handler_t   *ph;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);
    ph = cmcf->phase_engine.handlers;

    while (ph[r->phase_handler].checker) {
        rc = ph[r->phase_handler].checker(r, &ph[r->phase_handler]);
        if (rc == NGX_OK) {
            return;
        }
    }
}
```

## 阶段处理器的返回值

- **NGX_OK**：处理完成，继续下一个阶段
- **NGX_DECLINED**：拒绝处理，继续下一个处理器
- **NGX_AGAIN**：需要异步处理，等待事件
- **NGX_DONE**：处理完成，结束请求
- **NGX_ERROR**：发生错误，结束请求
- **NGX_HTTP_xxx**：HTTP错误状态码

## 实际应用示例

```nginx
server {
    listen 80;
    server_name example.com;
    
    # POST_READ_PHASE: 设置真实IP
    set_real_ip_from 10.0.0.0/8;
    real_ip_header X-Forwarded-For;
    
    # SERVER_REWRITE_PHASE: 服务器级重写
    rewrite ^/old/(.*)$ /new/$1 last;
    
    location /api/ {
        # REWRITE_PHASE: location级重写
        rewrite ^/api/(.*)$ /backend/$1 break;
        
        # PREACCESS_PHASE: 限流
        limit_req zone=api burst=10 nodelay;
        
        # ACCESS_PHASE: 访问控制
        allow 192.168.1.0/24;
        deny all;
        
        # CONTENT_PHASE: 代理到后端
        proxy_pass http://backend;
    }
    
    # LOG_PHASE: 访问日志
    access_log /var/log/nginx/access.log;
}
```

## 总结

这种分阶段的架构使得nginx能够灵活地处理各种HTTP请求，每个阶段都有明确的职责，便于模块开发和维护。 