# MyTest HTTP模块编译和使用指南

## 模块功能

MyTest模块是一个简单的nginx第三方HTTP模块，具有以下功能：

1. **访问次数统计**：统计每个location的访问次数
2. **开关控制**：通过配置指令控制模块的启用/禁用
3. **内容输出**：生成美观的HTML页面显示统计信息

## 文件位置

模块源码位于：`mycode/ngx_http_mytest_module.c`

## 编译步骤

### 方法1：使用--add-module选项（推荐）

```bash
# 配置编译选项，指定模块路径
./configure --add-module=mycode

# 编译
make

# 安装（可选）
make install
```

### 方法2：手动集成到源码中

1. **复制模块文件**：
   ```bash
   cp mycode/ngx_http_mytest_module.c src/http/modules/
   ```

2. **修改auto/modules文件**：
   在`auto/modules`文件中添加：
   ```bash
   if [ $HTTP_MYTEST = YES ]; then
       have=NGX_HTTP_MYTEST . auto/have
       HTTP_SRCS="$HTTP_SRCS $HTTP_MYTEST_SRCS"
       HTTP_DEPS="$HTTP_DEPS $HTTP_MYTEST_DEPS"
       HTTP_INCS="$HTTP_INCS $HTTP_MYTEST_INCS"
   fi
   ```

3. **修改configure脚本**：
   在configure脚本中添加：
   ```bash
   HTTP_MYTEST=NO
   
   # 添加命令行选项
   --add-module=mycode)
       HTTP_MYTEST=YES
       ;;
   ```

4. **编译**：
   ```bash
   ./configure
   make
   ```

### 方法3：创建config文件（最简单）

在`mycode/config`文件中添加：
```bash
ngx_addon_name=ngx_http_mytest_module
HTTP_MODULES="$HTTP_MODULES ngx_http_mytest_module"
NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_mytest_module.c"
```

然后编译：
```bash
./configure --add-module=mycode
make
```

## 配置使用

### 1. 基本配置

在nginx.conf中添加配置：

```nginx
http {
    server {
        listen 80;
        server_name localhost;

        location /mytest {
            mytest_enabled on;  # 启用模块
        }
    }
}
```

### 2. 配置指令说明

- **mytest_enabled**: 控制模块开关
  - `on`: 启用模块
  - `off`: 禁用模块
  - 默认值: `off`

### 3. 访问测试

编译并启动nginx后，访问：
- `http://localhost/mytest` - 显示访问统计页面
- 每次访问都会增加计数器

## 模块特性

### 1. 线程安全

- 使用`ngx_atomic_t`类型确保访问计数的线程安全
- 支持多worker进程环境

### 2. 配置继承

- 支持在http、server、location三个层次配置
- 遵循nginx的配置继承规则

### 3. 性能优化

- 使用内存池分配内存
- 最小化内存分配次数
- 高效的字符串格式化

## 扩展功能

可以基于此模块扩展更多功能：

1. **持久化存储**：将访问次数保存到文件或数据库
2. **更多统计信息**：记录访问时间、IP地址等
3. **访问控制**：基于IP或用户名的访问限制
4. **API接口**：提供JSON格式的统计数据

## 故障排除

### 1. 编译错误

- 确保nginx源码版本兼容
- 检查头文件包含是否正确
- 验证函数声明和定义

### 2. 运行时错误

- 检查nginx错误日志
- 验证配置文件语法
- 确认模块正确加载

### 3. 功能异常

- 检查配置指令是否正确
- 验证location匹配规则
- 确认模块处理函数被正确注册

## 快速测试

使用提供的测试脚本：
```bash
chmod +x test_mytest_module.sh
./test_mytest_module.sh
``` 