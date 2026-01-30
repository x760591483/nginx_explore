## nginx安装

### 一、安装前准备

#### 0. 历史安装

./configure --help
sudo apt update
sudo apt install libpcre3 libpcre3-dev
sudo apt install zlib1g zlib1g-dev
sudo apt install -y libssl-dev
./configure --prefix=./build --with-debug  --with-http_ssl_module
make
make install


#### 1. 安装依赖包
```bash
# CentOS/RHEL
sudo yum install -y gcc gcc-c++ make pcre pcre-devel zlib zlib-devel openssl openssl-devel

# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential libpcre3 libpcre3-dev zlib1g zlib1g-dev libssl-dev
```

#### 2. 下载 nginx 源码
```bash
# 下载最新稳定版
wget http://nginx.org/download/nginx-1.24.0.tar.gz

# 解压
tar -zxvf nginx-1.24.0.tar.gz
cd nginx-1.24.0
```

---

### 二、编译配置参数详解

#### 1. 基本配置参数

```bash
./configure \
  --prefix=/usr/local/nginx \              # nginx安装目录
  --sbin-path=/usr/sbin/nginx \            # nginx可执行文件路径
  --conf-path=/etc/nginx/nginx.conf \      # 配置文件路径
  --error-log-path=/var/log/nginx/error.log \  # 错误日志路径
  --http-log-path=/var/log/nginx/access.log \  # 访问日志路径
  --pid-path=/var/run/nginx.pid \          # pid文件路径
  --lock-path=/var/run/nginx.lock \        # 锁文件路径
  --user=nginx \                           # 运行用户
  --group=nginx                            # 运行组
  --with-debug                        #使能日志
```

#### 2. 常用模块参数

##### HTTP 核心模块
```bash
--with-http_ssl_module              # HTTPS支持
--with-http_v2_module               # HTTP/2支持
--with-http_realip_module           # 获取真实IP
--with-http_addition_module         # 响应前后添加内容
--with-http_sub_module              # 字符串替换
--with-http_dav_module              # WebDAV支持
--with-http_flv_module              # FLV流媒体支持
--with-http_mp4_module              # MP4流媒体支持
--with-http_gunzip_module           # gzip解压
--with-http_gzip_static_module      # 预压缩文件支持
--with-http_random_index_module     # 随机首页
--with-http_secure_link_module      # 安全链接
--with-http_stub_status_module      # 状态监控
--with-http_auth_request_module     # 认证请求
--with-http_slice_module            # 文件切片
```

##### Stream 模块（TCP/UDP负载均衡）
```bash
--with-stream                       # 启用stream模块
--with-stream_ssl_module            # stream SSL支持
--with-stream_realip_module         # stream真实IP
--with-stream_ssl_preread_module    # SSL预读
```

##### 其他模块
```bash
--with-threads                      # 线程池支持
--with-file-aio                     # 异步IO
--with-pcre                         # PCRE库
--with-pcre-jit                     # PCRE JIT编译
--with-libatomic                    # 原子操作库
```

#### 3. 禁用模块参数
```bash
--without-http_charset_module       # 禁用charset模块
--without-http_gzip_module          # 禁用gzip模块
--without-http_ssi_module           # 禁用SSI模块
--without-http_userid_module        # 禁用userid模块
--without-http_access_module        # 禁用access模块
--without-http_auth_basic_module    # 禁用基本认证
--without-http_autoindex_module     # 禁用自动索引
--without-http_geo_module           # 禁用geo模块
--without-http_map_module           # 禁用map模块
--without-http_split_clients_module # 禁用split_clients模块
--without-http_referer_module       # 禁用referer模块
--without-http_rewrite_module       # 禁用rewrite模块
--without-http_proxy_module         # 禁用proxy模块
--without-http_fastcgi_module       # 禁用fastcgi模块
--without-http_uwsgi_module         # 禁用uwsgi模块
--without-http_scgi_module          # 禁用scgi模块
--without-http_memcached_module     # 禁用memcached模块
--without-http_limit_conn_module    # 禁用limit_conn模块
--without-http_limit_req_module     # 禁用limit_req模块
--without-http_empty_gif_module     # 禁用empty_gif模块
--without-http_browser_module       # 禁用browser模块
--without-http_upstream_hash_module # 禁用upstream_hash模块
--without-http_upstream_ip_hash_module      # 禁用ip_hash模块
--without-http_upstream_least_conn_module   # 禁用least_conn模块
--without-http_upstream_keepalive_module    # 禁用keepalive模块
--without-http_upstream_zone_module         # 禁用zone模块
```

---

### 三、添加自定义模块

#### 方法1: 编译时添加静态模块

```bash
./configure \
  --prefix=/usr/local/nginx \
  --add-module=/path/to/custom-module-1 \     # 添加第三方模块
  --add-module=/path/to/custom-module-2 \     # 可添加多个模块
  --with-http_ssl_module
```

#### 方法2: 编译动态模块（nginx 1.9.11+）

```bash
./configure \
  --prefix=/usr/local/nginx \
  --add-dynamic-module=/path/to/custom-module \  # 动态模块
  --with-compat                                   # 兼容模式
```

动态模块在配置文件中加载：
```nginx
load_module modules/ngx_http_custom_module.so;
```

#### 常用第三方模块示例

##### 1. ngx_http_geoip2_module（地理位置）
```bash
# 下载模块
git clone https://github.com/leev/ngx_http_geoip2_module.git

# 编译
./configure \
  --prefix=/usr/local/nginx \
  --add-module=../ngx_http_geoip2_module
```

##### 2. ngx_cache_purge（缓存清除）
```bash
# 下载
git clone https://github.com/FRiCKLE/ngx_cache_purge.git

# 编译
./configure \
  --prefix=/usr/local/nginx \
  --add-module=../ngx_cache_purge
```

##### 3. nginx-module-vts（流量统计）
```bash
# 下载
git clone https://github.com/vozlt/nginx-module-vts.git

# 编译
./configure \
  --prefix=/usr/local/nginx \
  --add-module=../nginx-module-vts
```

##### 4. headers-more-nginx-module（更多头部操作）
```bash
# 下载
git clone https://github.com/openresty/headers-more-nginx-module.git

# 编译
./configure \
  --prefix=/usr/local/nginx \
  --add-module=../headers-more-nginx-module
```

##### 5. lua-nginx-module（Lua支持）
```bash
# 需要先安装LuaJIT
wget http://luajit.org/download/LuaJIT-2.0.5.tar.gz
tar -zxvf LuaJIT-2.0.5.tar.gz
cd LuaJIT-2.0.5
make && sudo make install

# 下载ngx_devel_kit和lua-nginx-module
git clone https://github.com/simpl/ngx_devel_kit.git
git clone https://github.com/openresty/lua-nginx-module.git

# 编译
export LUAJIT_LIB=/usr/local/lib
export LUAJIT_INC=/usr/local/include/luajit-2.0

./configure \
  --prefix=/usr/local/nginx \
  --add-module=../ngx_devel_kit \
  --add-module=../lua-nginx-module
```

---

### 四、完整编译安装流程

#### 1. 完整配置示例
```bash
./configure \
  --prefix=/usr/local/nginx \
  --sbin-path=/usr/sbin/nginx \
  --conf-path=/etc/nginx/nginx.conf \
  --error-log-path=/var/log/nginx/error.log \
  --http-log-path=/var/log/nginx/access.log \
  --pid-path=/var/run/nginx.pid \
  --lock-path=/var/run/nginx.lock \
  --user=nginx \
  --group=nginx \
  --with-http_ssl_module \
  --with-http_v2_module \
  --with-http_realip_module \
  --with-http_stub_status_module \
  --with-http_gzip_static_module \
  --with-pcre \
  --with-stream \
  --with-stream_ssl_module \
  --with-threads \
  --add-module=/path/to/custom-module
```

#### 2. 编译和安装
```bash
# 编译（使用多核加速）
make -j$(nproc)

# 安装
sudo make install
```

#### 3. 创建nginx用户
```bash
sudo useradd -r -M -s /sbin/nologin nginx
```

#### 4. 创建必要目录
```bash
sudo mkdir -p /var/log/nginx
sudo mkdir -p /etc/nginx/conf.d
sudo mkdir -p /var/cache/nginx/client_temp
sudo mkdir -p /var/cache/nginx/proxy_temp
sudo mkdir -p /var/cache/nginx/fastcgi_temp
sudo mkdir -p /var/cache/nginx/uwsgi_temp
sudo mkdir -p /var/cache/nginx/scgi_temp
```

#### 5. 配置 systemd 服务（推荐）
```bash
sudo vim /etc/systemd/system/nginx.service
```

添加以下内容：
```ini
[Unit]
Description=The nginx HTTP and reverse proxy server
After=network.target remote-fs.target nss-lookup.target

[Service]
Type=forking
PIDFile=/var/run/nginx.pid
ExecStartPre=/usr/sbin/nginx -t
ExecStart=/usr/sbin/nginx
ExecReload=/bin/kill -s HUP $MAINPID
ExecStop=/bin/kill -s QUIT $MAINPID
PrivateTmp=true

[Install]
WantedBy=multi-user.target
```

启用服务：
```bash
sudo systemctl daemon-reload
sudo systemctl enable nginx
sudo systemctl start nginx
sudo systemctl status nginx
```

---

### 五、查看编译参数

#### 查看已安装nginx的编译参数
```bash
nginx -V
```

这会显示：
- nginx版本
- 编译时使用的所有configure参数
- 编译器信息
- OpenSSL版本等

---

### 六、升级nginx（保留原有配置）

```bash
# 1. 下载新版本并解压
wget http://nginx.org/download/nginx-1.26.0.tar.gz
tar -zxvf nginx-1.26.0.tar.gz
cd nginx-1.26.0

# 2. 使用相同的configure参数（从nginx -V获取）
./configure [原有参数]

# 3. 编译（不要make install）
make -j$(nproc)

# 4. 备份旧版本
sudo mv /usr/sbin/nginx /usr/sbin/nginx.old

# 5. 复制新版本
sudo cp objs/nginx /usr/sbin/nginx

# 6. 测试配置
sudo nginx -t

# 7. 平滑升级
sudo kill -USR2 `cat /var/run/nginx.pid`
sudo kill -QUIT `cat /var/run/nginx.pid.oldbin`
```

---

### 七、常见问题

#### 1. 找不到PCRE库
```bash
# 指定PCRE路径
./configure --with-pcre=/path/to/pcre-8.45
```

#### 2. OpenSSL版本问题
```bash
# 指定OpenSSL路径
./configure --with-openssl=/path/to/openssl-1.1.1
```

#### 3. 查看模块是否加载成功
```bash
nginx -V 2>&1 | grep -o -- '--with-[^ ]*'
nginx -V 2>&1 | grep -o -- '--add-[^ ]*'
```

#### 4. 测试配置文件
```bash
sudo nginx -t
sudo nginx -T  # 显示完整配置
```

---

### 八、推荐的生产环境配置

```bash
./configure \
  --prefix=/usr/local/nginx \
  --sbin-path=/usr/sbin/nginx \
  --conf-path=/etc/nginx/nginx.conf \
  --error-log-path=/var/log/nginx/error.log \
  --http-log-path=/var/log/nginx/access.log \
  --pid-path=/var/run/nginx.pid \
  --lock-path=/var/run/nginx.lock \
  --user=nginx \
  --group=nginx \
  --with-http_ssl_module \
  --with-http_v2_module \
  --with-http_realip_module \
  --with-http_addition_module \
  --with-http_sub_module \
  --with-http_dav_module \
  --with-http_flv_module \
  --with-http_mp4_module \
  --with-http_gunzip_module \
  --with-http_gzip_static_module \
  --with-http_random_index_module \
  --with-http_secure_link_module \
  --with-http_stub_status_module \
  --with-http_auth_request_module \
  --with-http_slice_module \
  --with-stream \
  --with-stream_ssl_module \
  --with-stream_realip_module \
  --with-stream_ssl_preread_module \
  --with-threads \
  --with-file-aio \
  --with-pcre \
  --with-pcre-jit
```
