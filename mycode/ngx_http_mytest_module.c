/*
 * Copyright (C) 2024 MyTest Module
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* 模块配置结构体 */
typedef struct {
    ngx_flag_t  enabled;        /* 开关变量 */
    ngx_atomic_t access_count;  /* 访问计数器 */
} ngx_http_mytest_loc_conf_t;

/* 指令定义 */
static ngx_command_t  ngx_http_mytest_commands[] = {
    { ngx_string("mytest_enabled"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_mytest_loc_conf_t, enabled),
      NULL },

    ngx_null_command
};

/* 前向声明 */
static void *ngx_http_mytest_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_mytest_init(ngx_conf_t *cf);

/* 模块上下文结构体 */
static ngx_http_module_t  ngx_http_mytest_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_mytest_init,                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_mytest_create_loc_conf,       /* create location configuration */
    ngx_http_mytest_merge_loc_conf         /* merge location configuration */
};

/* 模块定义 */
ngx_module_t  ngx_http_mytest_module = {
    NGX_MODULE_V1,
    &ngx_http_mytest_module_ctx,           /* module context */
    ngx_http_mytest_commands,              /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

/* 创建location配置 */
static void *
ngx_http_mytest_create_loc_conf(ngx_conf_t *cf)
{
    printf("ngx_http_mytest_create_loc_conf --> \n");
    ngx_http_mytest_loc_conf_t  *conf;

    /* 添加配置创建日志 */
    ngx_log_error(NGX_LOG_INFO, cf->log, 0, 
                  "MyTest module: creating location configuration");

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_mytest_loc_conf_t));
    if (conf == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, 
                      "MyTest module: failed to allocate location configuration");
        return NULL;
    }

    /*
     * 设置默认值
     */
    conf->enabled = NGX_CONF_UNSET;
    conf->access_count = 0;

    ngx_log_error(NGX_LOG_INFO, cf->log, 0, 
                  "MyTest module: location configuration created successfully");

    return conf;
}

/* 合并location配置 */
static char *
ngx_http_mytest_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    printf("ngx_http_mytest_merge_loc_conf --> \n");
    ngx_http_mytest_loc_conf_t *prev = parent;
    ngx_http_mytest_loc_conf_t *conf = child;

    /* 添加配置合并日志 */
    ngx_log_error(NGX_LOG_INFO, cf->log, 0, 
                  "MyTest module: merging location configuration");

    ngx_conf_merge_value(conf->enabled, prev->enabled, 0);

    ngx_log_error(NGX_LOG_INFO, cf->log, 0, 
                  "MyTest module: location configuration merged, enabled=%d", 
                  (int)conf->enabled);

    return NGX_CONF_OK;
}

/* HTTP处理函数 */
static ngx_int_t
ngx_http_mytest_handler(ngx_http_request_t *r)
{
    printf("ngx_http_mytest_handler --> \n");
    ngx_http_mytest_loc_conf_t  *lcf;
    ngx_buf_t                    *b;
    ngx_chain_t                   out;
    ngx_uint_t                    count;
    u_char                        *p;
    size_t                        content_length;

    /* 添加调试日志 - 使用更高级别的日志 */
    ngx_log_error(NGX_LOG_ALERT, r->connection->log, 0, 
                  "=== MyTest module handler called for URI: %V ===", &r->uri);

    /* 获取模块配置 */
    lcf = ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);

    /* 检查模块是否启用 */
    if (!lcf->enabled) {
        printf("MyTest module is disabled,break \n");
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, 
                      "MyTest module is disabled");
        return NGX_DECLINED;
    }

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, 
                  "MyTest module is enabled, processing request");

    /* 增加访问计数 */
    count = ngx_atomic_fetch_add(&lcf->access_count, 1) + 1;
    printf("count = %d\n", (int)count);
    
    /* 设置响应头 */
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";

    /* 分配响应缓冲区 */
    b = ngx_create_temp_buf(r->pool, 1024);
    if (b == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                      "Failed to create temp buffer");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* 生成HTML内容 */
    p = ngx_snprintf(b->last, 1024,
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>MyTest Module</title>\n"
        "    <meta charset=\"utf-8\">\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; }\n"
        "        .container { max-width: 600px; margin: 0 auto; }\n"
        "        .header { background: #f0f0f0; padding: 20px; border-radius: 5px; }\n"
        "        .content { margin-top: 20px; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }\n"
        "        .count { font-size: 24px; color: #007cba; font-weight: bold; }\n"
        "        .info { margin-top: 15px; color: #666; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <div class=\"container\">\n"
        "        <div class=\"header\">\n"
        "            <h1>MyTest HTTP Module</h1>\n"
        "            <p>这是一个简单的nginx第三方模块示例</p>\n"
        "        </div>\n"
        "        <div class=\"content\">\n"
        "            <h2>访问统计</h2>\n"
        "            <p>当前访问次数: <span class=\"count\">%ui</span></p>\n"
        "            <div class=\"info\">\n"
        "                <p><strong>模块功能:</strong></p>\n"
        "                <ul>\n"
        "                    <li>访问次数统计</li>\n"
        "                    <li>开关控制</li>\n"
        "                    <li>简单的内容输出</li>\n"
        "                </ul>\n"
        "                <p><strong>配置指令:</strong> mytest_enabled on|off</p>\n"
        "            </div>\n"
        "        </div>\n"
        "    </div>\n"
        "</body>\n"
        "</html>\n",
        count);

    /* 正确设置缓冲区结束位置和计算内容长度 */
    content_length = p - b->last;
    printf("content_length = %d\n", (int)content_length);
    b->last = p;

    /* 设置缓冲区标志 - 这是关键修复 */
    b->last_buf = 1;  /* 标记这是最后一个缓冲区 */
    b->last_in_chain = 1;  /* 标记这是链中的最后一个缓冲区 */

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, 
                  "Generated HTML content, length: %d", (int)content_length);

    /* 设置Content-Length头 */
    r->headers_out.content_length_n = content_length;

    /* 设置响应体 */
    out.buf = b;
    out.next = NULL;

    /* 发送响应头 */
    r->header_only = 0;
    if (ngx_http_send_header(r) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
                      "Failed to send header");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    printf("send header AAA\n");
    
    /* 发送响应体 */
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, 
                  "Sending response body, content_length: %d", (int)content_length);
    ngx_int_t rc = ngx_http_output_filter(r, &out);
    printf("send body BBB\n");
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, 
                  "Response sent, return code: %d", rc);
    printf("send body CCC: %d\n", (int)rc);
    
    /* 确保响应完成 */
    if (rc == NGX_OK) {
        rc = NGX_HTTP_OK;
    }
    
    return rc;
}

/* 模块初始化函数 ngx_http_mytest_init 是模块的后配置初始化函数，主要作用是：
注册HTTP处理函数：将模块的处理函数注册到nginx的HTTP处理阶段 
插入处理链：将 ngx_http_mytest_handler 添加到 NGX_HTTP_CONTENT_PHASE 阶段的处理器数组中
这个函数在nginx启动时被调用，用于注册模块的处理函数到HTTP处理阶段*/
static ngx_int_t
ngx_http_mytest_init(ngx_conf_t *cf)
{
    printf("ngx_http_mytest_init --> \n");
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    /* 添加初始化日志 - 使用更高级别的日志 */
    ngx_log_error(NGX_LOG_ALERT, cf->log, 0, 
                  "=== MyTest module initialization started ===");

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    /* 添加调试日志 */
    ngx_log_error(NGX_LOG_ALERT, cf->log, 0, 
                  "MyTest module: got core main conf, registering handler to CONTENT_PHASE");

    /* 将处理函数插入到数组的开头，确保优先执行 */
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        ngx_log_error(NGX_LOG_ALERT, cf->log, 0, 
                      "MyTest module: failed to push handler to CONTENT_PHASE");
        return NGX_ERROR;
    }

    *h = ngx_http_mytest_handler;
    
    ngx_log_error(NGX_LOG_ALERT, cf->log, 0, 
                  "MyTest module: handler registered at position %d", 
                  (int)(cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers.nelts - 1));

    /* 添加成功日志 */
    ngx_log_error(NGX_LOG_ALERT, cf->log, 0, 
                  "=== MyTest module initialization completed successfully ===");

    return NGX_OK;
}
