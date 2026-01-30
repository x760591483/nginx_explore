





gdb ./build/sbin/nginx

set follow-fork-mode child
set detach-on-fork off


b ngx_http_init_connection
b ngx_http_process_request_line
b ngx_http_handler
b ngx_http_core_content_phase
b ngx_http_static_handler


----------------------------------------------------------------------------------

## Nginx 运行后 GDB 调试输出解析

当 nginx 进行 run 后打印内容如下：

### 1. 设置断点阶段

```gdb
Breakpoint 1 at 0x67107: file src/http/ngx_http_request.c, line 208.
```
**说明**: 第一个断点设置成功，位于 HTTP 请求初始化函数 `ngx_http_init_connection`，内存地址 0x67107

```gdb
(gdb) b ngx_http_process_request_line
Breakpoint 2 at 0x6ab8b: file src/http/ngx_http_request.c, line 1063.
```
**说明**: 第二个断点设置在 HTTP 请求行处理函数，用于解析 HTTP 请求的第一行（方法、URI、版本）

```gdb
(gdb) b ngx_http_handler
Breakpoint 3 at 0x6031d: file src/http/ngx_http_core_module.c, line 821.
```
**说明**: 第三个断点设置在 HTTP 核心处理器入口函数，这是 HTTP 请求处理的主入口

### 2. 启动程序阶段

```gdb
(gdb) r
Starting program: /home/xxh/mynginx/nginx_explore/build/sbin/nginx
```
**说明**: 执行 `run` 命令启动 nginx 程序

```gdb
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
```
**说明**: GDB 启用线程调试支持，使用系统的 libthread_db 库来跟踪多线程程序

### 3. 进程 Fork 阶段

```gdb
[Attaching after Thread 0x7ffff77d1740 (LWP 15969) fork to child process 15972]
[New inferior 2 (process 15972)]
```
**说明**: Nginx master 进程（PID 15969）fork 出第一个 worker 进程（PID 15972），GDB 自动附加到子进程

```gdb
Reading symbols from /usr/lib/debug/.build-id/4f/7b0c955c3d81d7cac1501a2498b69d1d82bfe7.debug...
Reading symbols from /usr/lib/debug/.build-id/ac/af96d7b1a6bad57b559d646233d5dc1a23257c.debug...
```
**说明**: 加载系统库的调试符号（libc 等），用于更好的调试体验

```gdb
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
```
**说明**: 为子进程 15972 启用线程调试支持

```gdb
[Attaching after Thread 0x7ffff77d1740 (LWP 15972) fork to child process 15973]
[New inferior 3 (process 15973)]
```
**说明**: Master 进程继续 fork 出第二个 worker 进程（PID 15973），GDB 再次附加到新的子进程

```gdb
Reading symbols from /home/xxh/mynginx/nginx_explore/build/sbin/nginx...
Reading symbols from /usr/lib/debug/.build-id/4f/7b0c955c3d81d7cac1501a2498b69d1d82bfe7.debug...
Reading symbols from /usr/lib/debug/.build-id/ac/af96d7b1a6bad57b559d646233d5dc1a23257c.debug...
```
**说明**: 为进程 15973 加载 nginx 程序及系统库的调试符号

```gdb
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
```
**说明**: 为子进程 15973 启用线程调试支持

### 4. 命中断点阶段

```gdb
[Switching to Thread 0x7ffff77d1740 (LWP 15973)]

Thread 3.1 "nginx" hit Breakpoint 1, ngx_http_init_connection (c=0x7ffff77961e0) at src/http/ngx_http_request.c:208
```
**说明**: 
- GDB 切换到线程 15973（worker 进程）
- 命中第一个断点 `ngx_http_init_connection`，说明有新的 HTTP 连接建立
- 参数 `c=0x7ffff77961e0` 是连接对象的内存地址
- 此时程序停在 `src/http/ngx_http_request.c` 文件的第 208 行

---

**总结**: Nginx 采用 master-worker 多进程模型，master 进程负责管理，worker 进程负责处理实际请求。这里可以看到创建了 2 个 worker 进程（15972 和 15973），并且其中一个 worker 进程接收到连接后触发了第一个断点。