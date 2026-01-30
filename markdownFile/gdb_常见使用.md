## 创建命令方式

### 多进程调试设置
- `set follow-fork-mode child` - 设置 fork 后跟踪子进程（默认跟踪父进程）
  - `parent` - 跟踪父进程（默认）
  - `child` - 跟踪子进程
- `set detach-on-fork off` - fork 后不分离其他进程，允许同时调试父子进程
  - `on` - fork 后分离不调试的进程（默认）
  - `off` - 保持对所有进程的控制，可使用 `inferior` 命令切换

## 断点操作

### 添加断点
- `break 函数名` 或 `b 函数名` - 在函数入口处设置断点
- `break 文件名:行号` 或 `b 文件名:行号` - 在指定文件的指定行设置断点
- `break 行号` - 在当前文件的指定行设置断点
- `break *地址` - 在指定内存地址设置断点
- `tbreak 位置` - 设置临时断点（只触发一次）

### 查看断点
- `info breakpoints` 或 `info b` - 查看所有断点信息
- `info break 断点号` - 查看指定断点的详细信息

### 清除断点
- `delete` 或 `d` - 删除所有断点
- `delete 断点号` 或 `d 断点号` - 删除指定编号的断点
- `clear 函数名` - 删除指定函数的断点
- `clear 行号` - 删除指定行的断点
- `disable 断点号` - 禁用断点（不删除）
- `enable 断点号` - 启用断点


