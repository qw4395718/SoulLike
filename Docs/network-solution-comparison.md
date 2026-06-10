# 联机方案选型参考

> 用途：SoulLike 项目在不同联机阶段的技术方案对比与选型记录
> 当前采用：Python Socket 中间服务 + UE4 FSocket

---

## 方案总览

| 方案 | 场景 | 复杂度 | 维护成本 | 适合阶段 |
|------|------|--------|---------|---------|
| A: 单服 PIE | 单进程调试 | 低 | 无 | Phase 1（已完成） |
| B: Python Socket 中间服务 | 本地双进程联调 | 低 | 低 | Phase 2（当前） |
| C: UE4 HTTP Server | 同机/内网通信 | 中 | 中 | 备选 |
| D: EOS P2P + Lobby | 公网联机 | 高 | 高 | 远期 |
| E: Steam OnlineSubsystem | Steam 平台联机 | 中 | 中 | 远期 |
| F: Advanced Sessions 插件 | 快速 Session 匹配 | 低 | 低 | 不适用 |

---

## 方案详述

### A: 单服 PIE 调试

状态：已完成（Phase 1）
适用阶段：基础逻辑验证

- 一个 UE4 进程，监听服务器 + 多个客户端
- 所有玩家在同一张地图
- 无法模拟"跨世界"

### B: Python Socket 中间服务（当前采用）

状态：开发中（Phase 2）
代码位置：MatchServer/match_server.py
UE4 端：SL_MatchClientSubsystem（GameInstanceSubsystem）

优势：
- 275 行 Python，纯逻辑无框架依赖
- UE4 侧 FSocket + JSON，内置 API 不加第三方依赖
- 协议和数据格式完全自主控制
- 调试直观：打印日志看消息流转

局限：
- 只有 TCP Socket 一层，无认证加密
- 不能用于公网（NAT 穿透问题）
- 单线程模型，高并发需改造

### C: UE4 HTTP Server（Experimental）

UE4 4.26 内置实验性模块，通过插件启用。内置了一个轻量 HTTP 服务端，可以直接在 UE4 进程内起一个 HTTP 服务接收 RESTful 请求。

对比 TCP Socket：

| 对比项 | TCP Socket | HTTP Server |
|--------|-----------|-------------|
| 协议 | 二进制/文本 | HTTP REST |
| 状态 | 全双工 | 请求-响应 |
| UE4 内建程度 | FSocket 基础 | 实验性模块 |
| 推送能力 | 双向即时推送 | 轮询或长轮询 |
| 稳定性 | UE4 稳定 API | 4.26 实验性 |

结论：当前 Phase 2 不需要 HTTP 的 RESTful 特性，Socket 更轻量。如果未来需要暴露 HTTP 接口给 Web 工具，可以在 Python 端用 Flask 加一层 HTTP 包装。

### D: EOS（Epic Online Services）

状态：远期备选
触发条件：需要公网联机 / 跨平台

Epic 官方的免费联机方案，覆盖 EOS Lobby、EOS P2P、EOS Sessions、EOS Player Data Storage。

对你的召唤系统的映射：
- 召唤标记 → Lobby 的自定义属性
- 标记查询 → SearchLobbies（按属性过滤）
- 召唤请求 → Lobby 聊天消息
- 灵体数据传输 → P2P 自定义消息通道

切换时机：
- 需要公网联机 → 考虑 EOS P2P
- 需要跨平台 → 考虑 EOS Lobby + P2P
- 需要对接平台账号/成就 → OnlineSubsystemSteam / EOS
- 只在局域网联调 → Python Socket 方案完全够用

### E: Steam OnlineSubsystem

状态：远期备选
适用场景：项目需要上架 Steam

UE4 内置的 OnlineSubsystemSteam 插件，配置 APP ID 后即可使用 Steam 的联机功能。

能力：
- Steam 好友邀请
- Steam 大厅
- Steam 成就/统计

局限：
- 所有玩家必须有 Steam 账号
- 必须在 Steam 环境下运行
- 功能绑定 Steam 生态

结论：如果项目最终上架 Steam，可以接 Steam 的成就/统计/好友邀请，但召唤系统的中间层继续用独立 Socket 服务做，不绑定平台 SDK。

### F: Advanced Sessions 插件

社区插件，封装了 OnlineSubsystem 的 Session 操作，提供蓝图层。

不适用于召唤系统的原因：Advanced Sessions 解决的是"玩家匹配进同一个 Session"的问题，而召唤系统的核心交互发生在两个已经在各自 Session 中的玩家之间。用 Session 方案来实现召唤系统需要大量 hack。

---

## 演进路线

```
Phase 1                     Phase 2                     远期
单服 PIE    ──────────→    Python Socket     ──────────→  EOS P2P
（已完成）                 （当前）                        （替换 Socket 层）
                                                          │
                                              需要 Steam？└→ OnlineSubsystemSteam
                                                  （只接成就/好友，中间层不动）
```

核心设计原则：联机通信层和数据格式层分离。无论底层的 Socket 换成 EOS P2P 还是 Steam，JSON 协议不变，UE4 侧的 MatchClientSubsystem 接口不变。

---

> 文档版本：v1.0
> 最后更新：2026-06-10
> 适用项目：SoulLike (UE 4.26)
