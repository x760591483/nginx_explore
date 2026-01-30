# MyTest HTTP模块项目

这是一个简单的nginx第三方HTTP模块示例，演示了如何开发nginx模块的基本流程。

## 📁 项目结构

```
nginx_explore/
├── mycode/
│   ├── ngx_http_mytest_module.c    # 模块源码
│   └── config                       # 模块配置文件
├── notes/
│   ├── mytest_module_config_example.conf    # 配置示例
│   └── mytest_module_build_guide.md         # 编译指南
├── test_mytest_module.sh           # 功能测试脚本
├── build_and_test.sh               # 编译和测试脚本
└── README_MyTest_Module.md         # 项目说明文档
```

## 🚀 快速开始

### 1. 编译模块

```bash
# 方法1：使用自动化脚本（推荐）
./build_and_test.sh

# 方法2：手动编译
./configure --add-module=mycode
make
```

### 2. 配置nginx

在nginx配置文件中添加：

```nginx
location /mytest {
    mytest_enabled on;  # 启用模块
}
```

### 3. 测试访问

访问 `http://localhost:8080/mytest` 查看效果。

## 🎯 模块功能

### 核心特性

1. **访问次数统计**
   - 使用原子操作确保线程安全
   - 支持多worker进程环境
   - 实时显示访问计数

2. **开关控制**
   - 通过 `mytest_enabled` 指令控制
   - 支持 on/off 两种状态
   - 默认状态为 off

3. **美观输出**
   - 生成响应式HTML页面
   - 包含CSS样式
   - 显示模块信息和统计

### 配置指令

| 指令 | 语法 | 默认值 | 说明 |
|------|------|--------|------|
| mytest_enabled | `mytest_enabled on\|off;` | off | 控制模块开关 |

### 使用示例

```nginx
# 启用模块
location /mytest {
    mytest_enabled on;
}

# 禁用模块
location /mytest_disabled {
    mytest_enabled off;
}

# 默认禁用（不配置指令）
location /mytest_default {
    # 模块默认禁用
}
```

## 🔧 开发说明

### 模块结构

1. **配置结构体** (`ngx_http_mytest_loc_conf_t`)
   - `enabled`: 开关标志
   - `access_count`: 访问计数器

2. **模块上下文** (`ngx_http_module_t`)
   - 定义配置创建和合并函数
   - 注册初始化函数

3. **指令定义** (`ngx_command_t`)
   - 定义配置指令
   - 指定解析函数

4. **处理函数** (`ngx_http_mytest_handler`)
   - 处理HTTP请求
   - 生成响应内容

### 关键函数

- `ngx_http_mytest_create_loc_conf()`: 创建配置
- `ngx_http_mytest_merge_loc_conf()`: 合并配置
- `ngx_http_mytest_handler()`: 请求处理
- `ngx_http_mytest_init()`: 模块初始化

## 📚 学习资源

### 相关文档

- [nginx模块开发指南](notes/mytest_module_build_guide.md)
- [配置示例](notes/mytest_module_config_example.conf)
- [nginx基础数据结构](notes/nginx基础数据.md)
- [HTTP框架11阶段](notes/http框架11阶段.md)

### 扩展方向

1. **持久化存储**
   - 将访问次数保存到文件
   - 集成数据库存储

2. **更多统计信息**
   - 访问时间记录
   - IP地址统计
   - 用户代理分析

3. **访问控制**
   - IP白名单/黑名单
   - 用户认证
   - 访问频率限制

4. **API接口**
   - JSON格式输出
   - RESTful API设计
   - 统计数据查询

## 🐛 故障排除

### 常见问题

1. **编译失败**
   - 检查nginx版本兼容性
   - 验证头文件包含
   - 确认函数声明正确

2. **模块不工作**
   - 检查配置文件语法
   - 确认模块正确加载
   - 查看错误日志

3. **访问计数不准确**
   - 检查多进程配置
   - 验证原子操作使用
   - 确认内存分配正确

### 调试技巧

1. **查看nginx错误日志**
   ```bash
   tail -f logs/error.log
   ```

2. **检查模块加载**
   ```bash
   ./objs/nginx -T 2>&1 | grep mytest
   ```

3. **测试配置语法**
   ```bash
   ./objs/nginx -t -c test_nginx.conf
   ```

## 📄 许可证

本项目仅用于学习和演示目的。

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个模块。

---

**注意**: 这是一个教学示例，生产环境使用前请进行充分测试。 