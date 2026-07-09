# SoulLike 项目：UE4.26 → UE5.8 升级指南

> 适用项目：D:\Code\SoulLike
> 生成日期：2026-07-09
> 目标引擎：UE 5.8

---

## 一、升级概要

本指南基于对项目完整扫描后编写。项目从 UE4.26 直接升级到 UE5.8，跨度较大（跨越 UE4.27、UE5.0~5.7），需要分阶段推进。

### 项目依赖概况

| 组件 | 版本/状态 | 升级影响 |
|------|----------|---------|
| 引擎内置模块 | Core, Engine, UMG, Slate | 少量 API 弃用，需按编译错误修正 |
| GameplayAbilities | 内置插件 | GAS 在 UE5 中 API 基本兼容 |
| UnLua (Tencent) | v2.3.6 | 代码层已有 UE5 宏分叉（`UE_5_0_OR_LATER`），需确认 UE5.8 兼容性 |
| UnLuaExtensions (LuaSocket/Protobuf/Rapidjson) | 随 UnLua | 同样需确认 |
| ALS v4 (Advanced Locomotion) | 纯内容资产 | 需在 UE5.8 重新保存 + 检查 Chaos 物理兼容性 |
| MatchServer | Python (独立) | 不受影响。代码在 `MatchServer/` 下 |
| Lua 脚本 | `Content/Script/` | 需检查 Lua API 兼容性，但 UE5 的 UnLua 接口变化不大 |

### 升级风险等级

- **高**：物理系统 (PhysX → Chaos) → 影响 ALS 角色移动、碰撞、布料模拟
- **中**：构建配置 (BuildSettings, Target.cs) → 编译系统语法变化
- **中**：第三方插件兼容性 → UnLua 需单独测试
- **低**：C++ API 变更 → 大多数 API 在编译错误中可逐步修正
- **低**：Python 匹配服务 → 完全独立，无需修改

---

## 二、升级前准备

### 2.1 备份

```bash
# 确保当前代码库所有更改已提交
git status
git add -A
git commit -m "[backup] pre-UE5.8-upgrade snapshot"

# 创建独立的备份分支
git checkout -b backup/pre-ue5.8
git push origin backup/pre-ue5.8
git checkout main
```

### 2.2 清理中间文件

```bash
# 清理 UE4.26 的中间产物（必须，否则 UE5 会误读）
rm -rf Intermediate
rm -rf Binaries
rm -rf DerivedDataCache
rm -rf .vs
rm -rf Saved
rm -rf Plugins/UnLua/Intermediate
rm -rf Plugins/UnLua/Binaries
rm -f SoulLikeDemo.sln
```

### 2.3 确认工具链

- 安装 **VS2022** (Community/Professional/Enterprise)
- 安装 **.NET Framework 4.6.2 SDK** 或更高版本（UE5 构建脚本需要）
- 确保 `git` LFS 已启用（`git lfs install`），资产文件可能很大
- 确认 UE5.8 已通过 Epic Games Launcher 安装

---

## 三、升级步骤（分阶段执行）

### 阶段 A：项目配置文件升级

#### A1. 修改 `.uproject` 文件

编辑 `SoulLikeDemo.uproject`，将 `EngineAssociation` 改为 `"5.8"`，可选补充 `Category` 和 `Description`。

#### A2. 修改 Target.cs 文件

**Source/SoulLikeDemo.Target.cs**：

```csharp
using UnrealBuildTool;

public class SoulLikeDemoTarget : TargetRules
{
    public SoulLikeDemoTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.AddRange(new string[] { "SoulLikeDemo" });
    }
}
```

**Source/SoulLikeDemoEditor.Target.cs**：同样修改，Type 改为 `TargetType.Editor`。

> 关键变更：
> - `DefaultBuildSettings` 从 V2 升级到 V5（V2 在 UE5 中已弃用）
> - 新增 `IncludeOrderVersion = EngineIncludeOrderVersion.Latest`

#### A3. 修改 Build.cs

在 `Source/SoulLikeDemo/SoulLikeDemo.Build.cs` 中添加 UE5.2+ 的 IWYU 兼容分支：

```csharp
#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.None;
#else
        bEnforceIWYU = false;
#endif
```

其余模块依赖列表保持不变。

#### A4. 更新 Config 文件

- `DefaultEngine.ini` 中的 Android 配置段（`AndroidRuntimeSettings`）建议在首次打开编辑器后重新生成
- `DefaultGame.ini` 中的打包配置（`ProjectPackagingSettings`）建议重新生成，手动迁移 `DirectoriesToAlwaysStageAsUFS` 和 `DirectoriesToAlwaysCook` 配置

---

### 阶段 B：打开 UE5.8 编辑器并让引擎转换项目

1. 双击 `SoulLikeDemo.uproject` → 如果关联已指向 UE5.8，会弹出引擎版本转换提示，选择 **"Yes"**
2. 如果提示需要重新编译模块，等待引擎编译 DLL
3. 如果 UE5.8 弹出 "Missing module" 或 "Plugin not compatible" 错误，进入阶段 C 处理
4. 首次成功打开后，直接关闭编辑器 → 再次打开确认无报错

---

### 阶段 C：插件兼容性处理

#### C1. UnLua 插件

当前 UnLua v2.3.6 的 Build.cs 已包含 `UE_5_0_OR_LATER` 和 `UE_5_2_OR_LATER` 宏分支，理论上可在 UE5.x 编译。但：

- **推荐操作**：从 [UnLua GitHub](https://github.com/tencent/UnLua) 检查是否有 UE5.8 的适配版本
- 如果 v2.3.6 在 UE5.8 编译失败，下载最新 release 覆盖 `Plugins/UnLua/` 目录

#### C2. UnLuaExtensions

- LuaSocket、LuaProtobuf、LuaRapidjson 通常在 UnLua 主库更新后同步更新
- 如果编译报错，暂时禁用这些扩展（在 `.uproject` 中移除对应启用配置）

---

### 阶段 D：C++ 编译修复（核心工作）

打开项目后，编辑器会自动尝试编译 C++ 代码。以下列举本项目可能遇到的编译错误及修复方案：

#### D1. `UAssetManager::GetStreamableManager()` 弃用

**受影响文件**：
- `Source/SoulLikeDemo/SoulLikeDemo.cpp`
- `Source/SoulLikeDemo/Private/UMG/HUD/HUD_ItemUseSlot.cpp`

**修复方案**：

```cpp
// UE4.26 旧写法
FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

// UE5.x 新写法
FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();
```

#### D2. 头文件路径变化

UE5.x 重构了部分头文件路径。编译错误中的 "file not found" 会明确指示缺失文件。常见情况：

- `#include "Animation/AnimBlueprintGeneratedClass.h"` → 可能需要改为 `#include "Animation/AnimClass.h"`
- `#include "AssetRegistryModule.h"` → 改为 `#include "AssetRegistry/AssetRegistryModule.h"`
- 如果报 `UAnimClass` 未定义，添加 `#include "Animation/AnimClass.h"`

#### D3. Chaos 物理适配

**项目当前无直接 PhysX API 调用**，所有物理操作通过引擎抽象 API（`SetSimulatePhysics`、`SetCollisionEnabled`、`FBodyInstance`）完成，理论上不需要修改 C++ 代码。

但 ALS v4 的内容资产是在 UE4.26 PhysX 时代制作的，需要注意：
- UE5 首次打开时会自动转换 PhysicsAsset 为 Chaos 格式
- 如果角色模拟物理后出现异常（布料、布娃娃），可能需要重新生成或微调 PhysicsAsset

#### D4. 核心模板 & 容器

`TMap`、`TArray`、`TSet`、`TSharedPtr`、`Cast<>`、`FText` 等核心模板和容器跨版本兼容，无需修改。

---

### 阶段 E：编译 & 迭代修复

#### E1. 首次编译

```bash
# 命令行编译（推荐用于查看完整错误日志）
"<UE5.8引擎路径>\Engine\Build\BatchFiles\Build.bat" SoulLikeDemoEditor Win64 Development -Project="D:\Code\SoulLike\SoulLikeDemo.uproject" -WaitMutex
```

#### E2. 推荐修复工作流

1. 编译 → 收集错误列表
2. 分批修复（每次 5~10 个同类错误）
3. 重新编译 → 继续修复下一批
4. 循环直到无编译错误

**每当遇到不确定的 API 时，先查询 UE5.8 的官方文档或引擎源码，不要凭空猜测。**

---

### 阶段 F：运行时验证

#### F1. 编辑器启动 & 资产重保存

- 成功编译后打开编辑器
- 检查 **Output Log** 是否有红色错误日志
- 右键 Content Browser 中的资产 → **Save All**（重点：ALS 动画蓝图、混合空间、物理资产）

#### F2. 动画 & 物理验证

- 运行游戏，检查 ALS 状态机过渡是否正常
- 检查根运动动画是否驱动角色移动
- 检查布娃娃/死亡动画是否正确

#### F3. GAS 系统验证

- 使用技能，检查 AttributeSet 数值变化
- 检查 GameplayEffect 持续时间和叠加规则
- 查看 `LogGameplayEffects:Verbose` 输出

#### F4. UI 验证

- 检查 HUD 控件显示是否正常
- 检查 WidgetComponent 的 Screen/World 空间渲染

#### F5. 网络验证

- 启动 MatchServer：`MatchServer/start_match_server.bat`
- 启动两个游戏实例，测试召唤标记流程

---

## 四、常见问题 FAQ

### Q1: 编译时出现大量 "undeclared identifier"

最可能的原因是 UE5.x 重构了头文件包含链。查看第一个编译错误定位根因，为缺失类型添加对应的 `#include`。

### Q2: ALS v4 角色不移动或动画异常

- 确认 `Project Settings → Physics` 使用 Chaos（UE5.8 默认）
- 在 UE5.8 中重新加载并保存 ALS 的物理资产
- 必要时参考 ALS-Community 的 UE5 迁移笔记：https://github.com/dyanikoglu/ALS-Community

### Q3: GAS 技能不激活

- 和 UE4.26 同样的排查路径：检查 ASC 初始化、GameplayTag 注册、Ability 的激活条件
- UE5.x 中 GAS 核心 API 无重大变更

### Q4: UnLua 脚本不执行

- 确认 UnLua 插件编译成功且在 UE5.8 中已启用
- 检查 Output Log 中的 Lua 错误信息
- 如果 UnLua 版本不兼容，从 GitHub 获取最新版

### Q5: Android 打包失败

- UE5.8 需要更新版本的 Gradle 和 Android SDK
- 在 Project Settings 中重新配置 Android 相关路径
- UE5.8 使用新的 Android 配置方式，旧的 `DefaultEngine.ini` 中的配置可能失效

---

## 五、升级完成后

1. 执行完整的全量测试（单机 + 网络 + Android）
2. 更新 `README.md` 中的引擎版本信息
3. 提交代码：`git add -A && git commit -m "[upgrade] UE4.26 -> UE5.8"`
4. 可选：迁移输入系统到 EnhancedInput（作为后续优化项）

---

## 六、需要我协助的部分

这份文档完成后，你可以：

1. 先阅读文档，对升级流程有整体认识
2. 完成阶段 A 的配置文件修改后，告诉我一声
3. 我会协助你：
   - 修改 `.uproject`、`Target.cs`、`Build.cs` 的具体代码
   - 解决编译错误（逐条分析并生成修复）
   - 处理 UnLua 插件的 UE5 兼容性问题
   - 调试 ALS 动画/物理异常
   - 验证 GAS 系统运行正常
