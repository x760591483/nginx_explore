
好，这一步**信息量很大**，而且说明了一件事：
👉 **你已经把“Windows → WSL 默认用户”设对了，但 VS Code 这边还在“记忆旧连接”**。

这不是你操作错，而是 **VS Code Remote-WSL 的一个典型缓存/会话问题**。

我一步一步带你**彻底掐断 root**。

---

## 先给你一句话结论

> **`wsl --set-default Ubuntu-22.04 --user xxh` 已经生效**
> 现在 VS Code 仍然显示 `root@...`，是因为：
>
> * VS Code **复用了之前以 root 身份建立的 WSL Server**
> * 没有重新拉起新的用户会话

---

## 第一件事：确认 WSL 本身已经不是 root（非常关键）

**不要开 VS Code**，先在 **PowerShell** 里：

```powershell
wsl --shutdown
wsl
```

进去后立刻执行：

```bash
whoami
```

### 你现在应该看到的是：

```
xxh
```

如果这里已经是 `xxh`
👉 **说明 WSL 层面已经完全正确了**

（如果这里还是 root，告诉我，我走另一条线）

---

## 第二件事：彻底让 VS Code“忘掉 root”

这是重点步骤，很多人会漏。

### 1️⃣ 完全关闭 VS Code（所有窗口）

* 确认右下角托盘里也没有 VS Code
* 实在不放心可以任务管理器结束 `Code.exe`

---

### 2️⃣ 清掉 Remote-WSL 的旧 Server（核心）

在 **PowerShell** 执行：

```powershell
wsl -d Ubuntu-22.04 -- rm -rf ~/.vscode-server
```

⚠️ 这一步 **不会删你的代码**
⚠️ 只是让 VS Code 重新安装 server

---

### 3️⃣ 用“正确用户”重新打开 VS Code

在 **WSL 终端（确认是 xxh）**：

```bash
cd ~/mynginx/nginx_explore
code .
```

这次你会看到：

* VS Code 弹出
* 左下角显示：

  ```
  >< WSL: Ubuntu-22.04
  ```
* 终端提示符变成：

  ```
  xxh@xxh:~/mynginx/nginx_explore$
  ```

---

## 第三件事：验证 Git（收官）

在 VS Code 终端里：

```bash
whoami
git status
```

✅ 不再有 `dubious ownership`
✅ 一切恢复正常

---

## 为什么会发生这种事？（你没做错）

简单说：

* 你 **最早一次** 是用 root 打开的：

  ```bash
  root → code .
  ```
* VS Code 就：

  * 在 `/root/.vscode-server` 装了 server
  * 后面一直复用
* 即使你后来改了 WSL 默认用户
* VS Code **不会自动切换身份**

👉 必须手动“清缓存 + 重连”

---

## 给你一个“以后绝对不踩坑”的口诀

> **WSL 默认用户改完以后：**
>
> 1️⃣ `wsl --shutdown`
> 2️⃣ 删 `~/.vscode-server`
> 3️⃣ 再 `code .`

---

## 如果你愿意（进阶但很有价值）

我可以顺手帮你做三件“终身受用”的事：

* 🔧 禁止 VS Code 以后再用 root（硬约束）
* 🧠 配好 `.bashrc` / `.profile`，一眼就能看出用户
* 🚀 给你一套 **WSL + VS Code + Git 的“零 root”规范**

你先按上面 **三步**来一遍，
然后把 **`whoami` 的输出 + VS Code 终端提示符**贴我，我帮你最终确认 💪



好，这个结果**非常关键**，而且现在可以**100% 定位问题了**。
你不是“没设置成功”，而是**你的 WSL 发行版压根不吃 `wsl --set-default … --user` 这一套**。

> **结论先给出：
> 对你这个 `wsl --import` 的 Ubuntu，
> 唯一、稳定、生效的方式是：`/etc/wsl.conf`。**

下面一步到位，不绕弯子。

---

## 为什么会这样？（你没做错）

你现在的事实是：

* ✔ `wsl --set-default Ubuntu-22.04 --user xxh` → **显示成功**
* ❌ `wsl` 进去 → `whoami` 仍然是 `root`

这说明：

> **Windows 端的“启动参数”已经改了，但这个发行版的 init 逻辑忽略了它**

这种情况 **只会出现在 `wsl --import` 的发行版**，而且并不少见。

👉 **官方兜底方案就是 `/etc/wsl.conf`**
👉 这是 WSL 自己启动时最优先读取的配置

---

## ✅ 终极解决方案（必定生效）

### 第 1 步：以 root 进入 WSL（你现在本来就是 root）

在 WSL 终端里，执行：

```bash
cat /etc/wsl.conf
```

大概率是：

```
cat: /etc/wsl.conf: No such file or directory
```

---

### 第 2 步：创建 `/etc/wsl.conf`

```bash
nano /etc/wsl.conf
```

写入 **下面这 4 行（一个字都别改）**：

```ini
[user]
default=xxh
```

保存并退出：

* `Ctrl + O`
* 回车
* `Ctrl + X`

---

### 第 3 步：**必须**完全关闭 WSL（关键）

在 **PowerShell** 里执行：

```powershell
wsl --shutdown
```

这一步**不做 = 100% 不生效**。

---

### 第 4 步：重新进入 WSL

```powershell
wsl
```

然后立刻验证：

```bash
whoami
```

### ✅ 你应该看到：

```
xxh
```

而不是 root。

---

## 如果这一步还不是 xxh（几乎不可能）

你把下面三条的输出**原样贴出来**：

```bash
cat /etc/wsl.conf
getent passwd xxh
ls -ld /home/xxh
```

我可以继续往 init 级别查，但一般到不了这一步。

---

## 接下来 **必须** 再做一件事（VS Code 相关）

因为你之前 **用 root 启动过 VS Code**，它已经“记住了 root”。

### 清掉旧的 VS Code Server（在 WSL 里）

```bash
rm -rf /root/.vscode-server
rm -rf /home/xxh/.vscode-server
```

然后：

```bash
cd ~/mynginx/nginx_explore
code .
```

VS Code 会重新安装 server，这次身份是 **xxh**。

---

## 最终你应该看到的 3 个“正确标志”

1️⃣ WSL 里：

```bash
whoami
# xxh
```

2️⃣ VS Code 终端：

```
xxh@xxh:~/mynginx/nginx_explore$
```

3️⃣ Git：

```bash
git status
# 正常，无 dubious ownership
```

---

## 给你一句非常重要的经验总结（你已经踩到核心）

> **凡是 `wsl --import` 的发行版：
> 不要信任何“命令行切用户”
> `/etc/wsl.conf` 才是最终裁判。**

你现在遇到的，已经是 **WSL 使用里的“高阶坑”**，新手基本走不到这里。

---

你先照我这个 **`/etc/wsl.conf → shutdown → whoami`** 的流程来一遍。
**只要你愿意贴结果，我可以陪你把这台 WSL 收拾到“职业级干净状态”。**
