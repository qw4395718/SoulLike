# UE4.26 Android 打包配置指南

> 适用项目：SoulLike（UE 4.26，Epic Launcher 安装）
> 目标设备：Xiaomi Redmi K50
> 文档整理时间：2026-05-23

---

## 目录

1. [环境准备](#1-环境准备)
2. [安装 JDK 8](#2-安装-jdk-8)
3. [安装 Android SDK / NDK](#3-安装-android-sdk--ndk)
4. [配置 UE4 编辑器](#4-配置-ue4-编辑器)
5. [项目 Android 参数设置](#5-项目-android-参数设置)
6. [打包 APK](#6-打包-apk)
7. [常见踩坑记录](#7-常见踩坑记录)

---

## 1. 环境准备

### 1.1 所需组件清单

| 组件 | 版本要求 | 用途 |
|------|---------|------|
| JDK | **1.8（8u492+）** | UE4 Android 打包必须 JDK 8 |
| Android SDK | **android-30** | 编译 Android 平台代码 |
| Android NDK | **r21e** | UE4.26 官方兼容版本 |
| SDK Build-Tools | **30.0.3** | APK 构建工具 |
| Platform-Tools | 最新 | adb 设备调试工具 |

### 1.2 文件结构规划

建议将 Android 开发工具统一放在非系统盘：

```
D:\Software\
  ├── Android\
  │     ├── cmdline-tools\latest\bin\    ← sdkmanager.exe
  │     ├── platforms\android-30\        ← SDK 平台
  │     ├── build-tools\30.0.3\           ← 构建工具
  │     ├── ndk\android-ndk-r21e\         ← NDK r21e
  │     └── platform-tools\               ← adb.exe
  └── Java\jdk1.8\                       ← JDK 8
```

---

## 2. 安装 JDK 8

### 2.1 下载

访问 Adoptium 官网下载 JDK 8 Windows x64 MSI：
https://adoptium.net/temurin/releases/?version=8

安装路径改为：`D:\Java\jdk1.8`

### 2.2 设置环境变量

系统变量 → 新建：

| 变量名 | 变量值 |
|--------|--------|
| JAVA_HOME | D:\Java\jdk1.8 |
| Path 追加 | %JAVA_HOME%\bin |

### 2.3 验证

```cmd
java -version
```

预期输出：`openjdk version "1.8.0_492"`

> ⚠️ **避坑（已踩）**：如果系统中同时安装了 JDK 17，java 命令会优先走 Path 里靠前的条目。执行 `where java` 查看顺序，删掉 JDK 17 的 Path 条目，不要只改 JAVA_HOME。

---

## 3. 安装 Android SDK / NDK

### 3.1 下载 Cmdline Tools

访问 https://developer.android.com/studio#command-line-tools-only

下载 commandlinetools-win-9477386_latest.zip

解压到：`D:\Software\Android\cmdline-tools`

### 3.2 修正目录结构

> ⚠️ **避坑（已踩）**：新版 Cmdline Tools 要求放在 `<sdk_root>\cmdline-tools\latest\` 下，不能直接放在 cmdline-tools\bin。

```cmd
cd /d D:\Software\Android\cmdline-tools
mkdir latest
move bin latest\bin
move lib latest\lib
move NOTICE.txt latest\
move source.properties latest\
```

最终结构：
```
D:\Software\Android\cmdline-tools\latest\bin\sdkmanager.exe
```

### 3.3 安装 SDK 组件

> ⚠️ **避坑（已踩）**：新版 sdkmanager 需要 JDK 11+ 才能运行。所以需要先临时切到 JDK 17 来跑以下命令，装完后切回 JDK 8。

```cmd
:: 临时切到 JDK 17
set JAVA_HOME=C:\Program Files\Microsoft\jdk-17.0.9-hotspot
set Path=%JAVA_HOME%\bin;%Path%

:: 安装组件
cd /d D:\Software\Android\cmdline-tools\latest\bin
set SKIP_JDK_VERSION_CHECK=true
sdkmanager "platforms;android-30"
sdkmanager "build-tools;30.0.3"
sdkmanager "ndk;21.4.7075529"
sdkmanager "platform-tools"

:: 切回 JDK 8
set JAVA_HOME=D:\Java\jdk1.8
set Path=%JAVA_HOME%\bin;%Path%
java -version   确认显示 1.8
```

### 3.4 确认 NDK 目录结构

sdkmanager 安装的 NDK 21.4.7075529 **与 UE4.26 完全兼容**，无需额外下载 r21e。

只需要确认 platforms 目录存在即可：

```cmd
dir D:\Software\Android\ndk\21.4.7075529\platforms
```

应该有 `android-16` 到 `android-30` 等子目录。

> ⚠️ **避坑（已踩）**：后续在 UE4 编辑器中配置 NDK API Level 时，一定要填 `android-21`（带前缀），不能只填 `21`。详见 4.2 节。

### 3.5 添加 Path

系统变量 → Path → 新建：

```
D:\Software\Android\platform-tools
D:\Software\Android\cmdline-tools\latest\bin
```

验证：

```cmd
adb version
```

---

## 4. 配置 UE4 编辑器

### 4.1 启动 UE4.26 → 打开项目

### 4.2 设置 SDK 路径

Edit → Project Settings → Platforms → Android SDK

| 字段 | 值 |
|------|-----|
| Location of Android SDK | D:\Software\Android |
| Location of Android NDK | D:\Software\Android\ndk\android-ndk-r21e |
| Java SDK (JDK) | D:\Java\jdk1.8 或 D:\Software\Eclipse Adoptium\jdk-8.0.492.9-hotspot |
| SDK API Level | android-30 |
| NDK API Level | **android-21** ← 注意带 android- 前缀 |

> ⚠️ **避坑（已踩）**：NDK API Level 不要只填 21，要填 android-21（匹配 NDK platforms 目录命名格式）。否则报错 The NDK API requested 'NDK r21' not installed。

填写后点击 **Accept SDK License**，路径应全部标绿。

---

## 5. 项目 Android 参数设置

Edit → Project Settings → Platforms → Android → APK Packaging

### 5.1 基本信息

| 字段 | 值 |
|------|-----|
| Android Package Name | com.SoulLike.SoulLikeDemo |
| Android Game Name | SoulLikeDemo |
| Store Version | 1 |
| Store Version (Used for APK) | 1.0.0 |

### 5.2 纹理压缩

勾选 **ASTC**（Redmi K50 支持，质量好体积小）。

### 5.3 SDK 版本

| 字段 | 值 |
|------|-----|
| Minimum SDK Version | 26 |
| Target SDK Version | 30 |

### 5.4 签名

开发阶段留空，UE4 自动使用 debug keystore。

### 5.5 游戏数据打包方式

在 **APK Packaging** 页面的 **Advanced** 中找到：

| 选项 | 推荐值 | 说明 |
|------|--------|------|
| **Package game data inside .apk** | **勾选 ✅** | 所有资源打进 APK，无需额外 obb 文件 |

> ⚠️ **重要**：不勾选会额外生成 .obb 文件，传输和安装都多一步。勾选后一个 APK 文件就能搞定。

---



## 6. 打包 APK

### 6.1 切换 JDK 8

打包前在命令提示符中确认：

```cmd
java -version
```

应显示 `1.8.0_xxx`。

### 6.2 在 UE4 编辑器中选择打包

**File → Package Project → Android → ASTC**

选取输出目录（建议：`D:\Code\SoulLike\AndroidBuild`）

### 6.3 等待编译完成

- C++ 编译：5-15 分钟
- Cook + 打包 APK：5-10 分钟
- 最终生成 SoulLikeDemo-Android-Shipping.apk

> 注意：打包期间不要关闭编辑器和命令行窗口。

### 6.4 Gradle 下载卡住处理

打包进度卡在 `Downloading https://services.gradle.org/distributions/gradle-6.1.1-all.zip` 是常见情况。Gradle 6.1.1 约 100MB，正常网速 3-5 分钟下完。如果长时间不动：

**方法一：手动下载 Gradle**

1. 浏览器打开 https://services.gradle.org/distributions/gradle-6.1.1-all.zip
2. 下载后查看缓存目录：
   ```cmd
   dir C:\Users\Administrator\.gradle\wrapper\dists\gradle-6.1.1-all\ /s
   ```
   会显示一个随机字符目录名，例如 `8ahg6...`。
3. 把 `gradle-6.1.1-all.zip`（**不要解压**）放到该目录下
4. 重新打包

**方法二：查看下载进度**

```cmd
:loop
dir C:\Users\Administrator\.gradle\wrapper\dists\gradle-6.1.1-all\ /s
timeout /t 2
goto loop
```
按 `Ctrl+C` 终止。文件大小在增长说明正在下载。

---

## 7. 常见踩坑记录

### ❌ 坑1：JDK 版本冲突

**现象**：java -version 显示 17.0.9，但已安装了 JDK 8。

**原因**：系统中存在 JDK 17 的 Path 条目排在 JDK 8 前面。

**解决**：`where java` 查看路径，删掉 JDK 17 的 Path 条目，只留 JDK 8。

---

### ❌ 坑2：sdkmanager 报 Could not determine SDK root

**现象**：
```
Error: Could not determine SDK root.
Error: Either specify it explicitly with --sdk_root= or move this package into its expected location: <sdk>\cmdline-tools\latest\
```

**原因**：Cmdline Tools 新版要求放在 `<sdk_root>\cmdline-tools\latest\`，不能直接 cmdline-tools\bin。

**解决**：
```cmd
cd <sdk_root>\cmdline-tools
mkdir latest
move bin latest\bin
move lib latest\lib
move NOTICE.txt latest\
move source.properties latest\
```

---

### ❌ 坑3：sdkmanager 需要 JDK 11+

**现象**：
```
Error: A JNI error has occurred...
java.lang.UnsupportedClassVersionError: com/android/sdklib/tool/sdkmanager/SdkManagerCli...
```

**原因**：新版 sdkmanager 编译目标为 JDK 11，JDK 8 无法运行。

**解决**：安装组件时临时切到 JDK 17，装完后切回 JDK 8 用于 UE4 打包。

---

### ❌ 坑4：（已排除）NDK 版本兼容性

**实际 NDK 21.4.7075529 完全兼容 UE4.26**，无需手动下载 r21e。
之前报错是因为 NDK API Level 填了 `21` 而非 `android-21`，详见坑5。

---

### ❌ 坑5：NDK API Level 命名格式

**现象**：明明 NDK 目录下有 platforms.json，UE4 仍然报错。

**原因**：UE4 编辑器中 NDK API Level 字段填了 21，实际需要 android-21。

**解决**：改为 android-21。

---

### ❌ 坑6：Android Clang 编译错误

**现象**：Windows Development 打包正常，Android 报类型转换错误或未定义标识符。

**原因**：Android 使用 Clang 编译器，比 MSVC 更严格。

**典型问题**：
- switch 表达式类型与 case 标签类型不匹配
- 使用了 Windows 专有宏（如 INT）

**解决**：修正类型匹配，用 static_cast<int32>() 替代平台宏。编译错误逐条修正即可。

---

### ❌ 坑7：META-INF 文件冲突（Entry name collided）

**现象**：
```
Entry name 'META-INF/android.arch.lifecycle_extensions.version' collided
```
打包中断，报资源合并冲突。

**原因**：
Android 打包时，Gradle/AAPT2 将多个 AAR/JAR 依赖解包并合并资源。不同依赖包（如不同版本的 Android Architecture Components）可能各自携带同名的 `META-INF` 文件。当版本不一致或同一依赖被多次引入时，AAPT2 检测到同名文件内容不一致，即报 collided。

UE4 将预处理过的安卓依赖缓存到 `Intermediate\Android` 目录。以下情况下该缓存会"过期"或产生冲突：

- 更新了插件（如联网/广告/社交插件引入了新版 AndroidX 依赖）
- 切换了 Git 分支，导致插件或依赖配置变更
- UE4 引擎或 SDK 在两次打包之间产生了不一致的缓存残留
- 旧的 `META-INF` 文件未被清理，与新下载的依赖版本不匹配

**解决**：

**方法一（最直接，已验证有效）**：

1. 关闭 UE4 编辑器
2. 删除 `Intermediate\Android` 目录（整个删）
3. 重新打开 UE4 编辑器，再次打包

UE4 检测到缓存不存在后会重新下载并展开所有安卓依赖，此时所有 `META-INF` 文件来自同一套配置，不会冲突。

**方法二（根治）**：

如果该问题频繁出现，需检查项目依赖中是否存在重复的 AndroidX / Support 生命周期库引用：

- 检查 `Build/Android/` 下的 Gradle 配置文件（如 `AppDeveloperConfig.xml`、`ProjectBuildBeforeGradle.xml`）
- 逐一排查插件目录下是否有 `.AAR` 或 `.JAR` 文件引入了 `android.arch.lifecycle`（旧版）和 `androidx.lifecycle`（新版）两套组件
- 如果某个插件已迁移到 AndroidX，移除旧版 `android.arch.*` 的依赖引用

> **建议**：开发迭代阶段，清 `Intermediate\Android` 是最省力的手段。上线打包前若仍有冲突，再用方法二根治。

---

## 环境变量速查表

| 变量名 | 值 |
|--------|-----|
| JAVA_HOME | D:\Java\jdk1.8 |
| ANDROID_HOME | D:\Software\Android |
| Path | %JAVA_HOME%\bin |
| Path | %ANDROID_HOME%\platform-tools |
| Path | %ANDROID_HOME%\cmdline-tools\latest\bin |

---

## 快速打包速查

```cmd
:: 1. 确认 JDK 版本
java -version
:: 必须是 1.8

:: 2. 在 UE4 编辑器中
File → Package Project → Android → ASTC

:: 3. APK 输出位置
D:\Code\SoulLike\AndroidBuild\SoulLikeDemo-Android-Shipping.apk
```

---

*完*