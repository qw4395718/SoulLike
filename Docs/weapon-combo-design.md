# SoulLike 武器连招设计方案

> 设计参考：[weapon-animsys-design.md](./weapon-animsys-design.md)
> 动画描述规范：4.5 节「方向+轨迹+部位/姿态」三元组

---

## 目录

1. [双剑（DualBlades）](#1-双剑dualblades)
   - [1.1 标签命名空间](#11-标签命名空间)
   - [1.2 轻击连段](#12-轻击连段)
   - [1.3 重击/蓄力](#13-重击蓄力)
   - [1.4 上挑腾空（地面→空中）](#14-上挑腾空地面空中)
   - [1.5 空中连段](#15-空中连段)
   - [1.6 空中重击/翻滚](#16-空中重击翻滚)
   - [1.7 特殊复合技](#17-特殊复合技)
   - [1.8 翻滚攻击](#18-翻滚攻击)
   - [1.9 非连招动作](#19-非连招动作)
   - [1.10 整体连招树](#110-整体连招树)
   - [1.11 动画文件重命名映射](#111-动画文件重命名映射)
   - [1.12 未使用动画（备选/扩展）](#112-未使用动画备选扩展)
2. [其他武器（预留）](#2-其他武器预留)

---

## 1 双剑（DualBlades）

### 1.1 标签命名空间

| 字段 | 值 |
|------|-----|
| 武器类型 | `EWeaponType::DualBlades` |
| 标签前缀 | `Dbl` |
| AnimSet 实例 | `WAS_TwinBlades` |
| 连招表实例 | `CT_TwinBlades` |

### 1.2 轻击连段

地面 4 段连招，左右交替挥斩，收尾大范围横扫。

#### 位置衔接链

```
Light 1(Attack13: 左上起双斜斩)
    ↓ 同侧右手位,双手身前
Light 2(Attack15: 跨两步右上起双斜斩)
    ↓ 同侧左手过渡
Light 3(Attack16: 跨两步左上起双斜斩)
    ↓ 同侧右手翻回
Light 4(Attack17: 右上起双斜斩接转身横扫)[终结]
```

#### 连招表

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | ExecuteType | DamageMult | StaminaCost |
|---|---|---|---|---|---|
| State.Window.None | Light | State.Combo.Dbl.Light_1 | Instant | 0.7 | 8 |
| State.Window.Dbl.Light_1_Release | Light | State.Combo.Dbl.Light_2 | Instant | 0.8 | 8 |
| State.Window.Dbl.Light_2_Release | Light | State.Combo.Dbl.Light_3 | Instant | 0.9 | 10 |
| State.Window.Dbl.Light_3_Release | Light | State.Combo.Dbl.Light_4_End | Instant | 1.2 | 14 |

#### 动作详情

| 段位 | OutputMontageTag | 源动画文件 | 三元组描述 | 起手→收尾 |
|------|-----------------|-----------|-----------|----------|
| Light 1 | State.Combo.Dbl.Light_1 | TwinSword_Attack13 | 左上起双斜斩 | 左上→右下 |
| Light 2 | State.Combo.Dbl.Light_2 | TwinSword_Attack15 | 跨两步右上起双斜斩 | 右上→左下 |
| Light 3 | State.Combo.Dbl.Light_3 | TwinSword_Attack16 | 跨两步左上起双斜斩 | 左上→右下 |
| Light 4 End | State.Combo.Dbl.Light_4_End | TwinSword_Attack17 | 右上起双斜斩接转身横扫 | 右上→横扫终 |

---

### 1.3 重击/蓄力

#### 待机起手重击

| OutputMontageTag | 源动画 | 三元组描述 | ExecuteType | DamageMult | 体力消耗 |
|-----------------|--------|-----------|------------|-----------|---------|
| State.Combo.Dbl.Heavy_Thrust | TwinSword_Attack25 | 右手蓄力前刺 | Charge | 1.5~2.0 | 20 |
| State.Combo.Dbl.Heavy_Thrust_L | TwinSword_Attack26 | 左手蓄力前刺 | Charge | 1.5~2.0 | 20 |
| State.Combo.Dbl.Heavy_Slash | TwinSword_Attack27 | 蓄力右上起双斜斩 | Charge | 1.8~2.8 | 25 |

#### 轻击派生重击

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | 位置匹配说明 |
|---|---|---|---|
| State.Window.Dbl.Light_1_Release | Heavy | State.Combo.Dbl.Heavy_Thrust | Light 1(13收双手身前) → 右手蓄力前刺 ✓ |
| State.Window.Dbl.Light_2_Release | Heavy | State.Combo.Dbl.Heavy_Thrust_L | Light 2(15收左手身后) → 左手蓄力前刺 ✓ |
| State.Window.Dbl.Light_3_Release | Heavy | State.Combo.Dbl.Heavy_Slash | Light 3(16收右手身后) → 蓄力右上双斜斩 ✓ |

---

### 1.4 上挑腾空（地面→空中）

腾空起手技，从轻击连段中派生，将敌人和自己带入空中状态。

| OutputMontageTag | 源动画 | 三元组描述 | 位置匹配 |
|-----------------|--------|-----------|---------|
| State.Combo.Dbl.Uppercut | TwinSword_Attack29 | 右手上挑进入腾空状态 | Light 1/3 收右下 → 右下起上挑 ✓ |
| State.Combo.Dbl.Uppercut_L | TwinSword_Attack30 | 左手上挑进入腾空状态 | Light 2 收左下 → 左下起上挑 ✓ |

#### 派生窗口

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | 说明 |
|---|---|---|---|
| State.Window.Dbl.Light_1_Release | Special | State.Combo.Dbl.Uppercut | Light 1(13收右转) → 右手上挑腾空 ✓ |
| State.Window.Dbl.Light_2_Release | Special | State.Combo.Dbl.Uppercut_L | Light 2(15收左手) → 左手上挑腾空 ✓ |
| State.Window.Dbl.Light_3_Release | Special | State.Combo.Dbl.Uppercut | Light 3(16收右手) → 右手上挑腾空 ✓ |

上挑腾空后进入空中状态，窗口 Tag 切换为 `State.Window.Dbl.Air_Entry`，后续接空中连段。

---

### 1.5 空中连段

腾空后轻击，四方向斜斩 + 竖劈终结，打满五段后自动落地。

#### 位置衔接链

```
上挑腾空 → Air_1(空右下) → Air_2(空左下) → Air_3(空左上) → Air_4(空右上) → Air_5(空竖劈→落地)
```

#### 连招表

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | 三元组描述 |
|---|---|---|---|
| State.Window.Dbl.Air_Entry | Light | State.Combo.Dbl.Air_1 | 空右下起双斜斩 |
| State.Window.Dbl.Air_1_Release | Light | State.Combo.Dbl.Air_2 | 空左下起双斜斩 |
| State.Window.Dbl.Air_2_Release | Light | State.Combo.Dbl.Air_3 | 空左上起双斜斩 |
| State.Window.Dbl.Air_3_Release | Light | State.Combo.Dbl.Air_4 | 空右上起双斜斩 |
| State.Window.Dbl.Air_4_Release | Light | State.Combo.Dbl.Air_5_End | 空双竖劈 |

#### 动作详情

| 段位 | OutputMontageTag | 源动画文件 |
|------|-----------------|-----------|
| Air 1 | State.Combo.Dbl.Air_1 | TwinSword_Air_Attack_1 |
| Air 2 | State.Combo.Dbl.Air_2 | TwinSword_Air_Attack_2 |
| Air 3 | State.Combo.Dbl.Air_3 | TwinSword_Air_Attack_3 |
| Air 4 | State.Combo.Dbl.Air_4 | TwinSword_Air_Attack_4 |
| Air 5 End | State.Combo.Dbl.Air_5_End | TwinSword_Air_Attack_5 |

---

### 1.6 空中重击/翻滚

空中按住重击触发翻滚竖劈，可持续 Loop，松手落地。

| OutputMontageTag | 源动画 | 三元组描述 | 用途 |
|-----------------|--------|-----------|------|
| State.Combo.Dbl.Air_Spin_Start | TwinSword_Air_Attack_6_Start | 空开始翻滚 | 空中重击起手 |
| State.Combo.Dbl.Air_Spin_Loop | TwinSword_Air_Attack_6_Loop | 连续翻滚竖劈 | 按住持续循环 |
| State.Combo.Dbl.Air_Spin_End | TwinSword_Air_Attack_6_End | 空到落地 | 松手/体力耗尽落地 |
| State.Combo.Dbl.Air_Spin | TwinSword_Air_Attack_6 | 空翻滚连续竖劈接落地 | 完整一套(按住到落地) |
| State.Combo.Dbl.Air_Spin_R | TwinSword_Attack32 | 右手连续腾转竖排 | 空中重击变体 |
| State.Combo.Dbl.Air_Spin_L | TwinSword_Attack31 | 左手连续腾转竖排 | 空中重击变体 |

#### 连招表

| ActiveRequireWindowTag | InputActionType | OutputMontageTag | ExecuteType | 说明 |
|---|---|---|---|---|
| State.Window.Dbl.Air_Entry | Heavy | State.Combo.Dbl.Air_Spin_Start | Instant | 空中状态→翻滚起手 |
| State.Window.Dbl.Air_1_Release | Heavy | State.Combo.Dbl.Air_Spin_Start | Instant | 空中轻击派生重击 |
| State.Window.Dbl.Air_2_Release | Heavy | State.Combo.Dbl.Air_Spin_Start | Instant | 空中轻击派生重击 |
| State.Window.Dbl.Air_3_Release | Heavy | State.Combo.Dbl.Air_Spin_Start | Instant | 空中轻击派生重击 |
| State.Window.Dbl.Air_4_Release | Heavy | State.Combo.Dbl.Air_Spin_Start | Instant | 空中轻击派生重击 |
| State.Window.Dbl.Air_Spin_Start_Release | Heavy | State.Combo.Dbl.Air_Spin_Loop | Channel | 按住→持续循环 |
| State.Window.Dbl.Air_Spin_Loop_Release | Heavy | State.Combo.Dbl.Air_Spin_Loop | Channel | 继续按住→持续循环 |
| State.Window.Dbl.Air_Spin_Loop_Release | Release | State.Combo.Dbl.Air_Spin_End | Instant | 松手→落地 |

#### 窗口过渡链

```
State.Combo.Dbl.Air_Spin_Start → State.Window.Dbl.Air_Spin_Start_Release
                          ├─ Heavy 按住 → State.Combo.Dbl.Air_Spin_Loop
                          │                  └─ State.Window.Dbl.Air_Spin_Loop_Release
                          │                       ├─ Heavy 继续按住 → State.Combo.Dbl.Air_Spin_Loop
                          │                       └─ Release 松手 → State.Combo.Dbl.Air_Spin_End →落地
                          └─ 无操作 → State.Combo.Dbl.Air_Spin → 直接落地
```

#### 空中重击流程

```
空中状态
  └─ Heavy → State.Combo.Dbl.Air_Spin_Start
                └─ (按住) → State.Combo.Dbl.Air_Spin_Loop [循环]
                               └─ (松手) → State.Combo.Dbl.Air_Spin_End → 落地
                └─ (不操作) → State.Combo.Dbl.Air_Spin → 自动落地
```

---

### 1.7 特殊复合技

整段动作包含起手→腾空→落地横扫，一次按键完成完整弧线。

| OutputMontageTag | 源动画 | 三元组描述 |
|-----------------|--------|-----------|
| State.Combo.Dbl.Special_RiseSweep | TwinSword_Attack19 | 左手右下起双斜斩腾空接落地横扫 |
| Dbl.Special_RiseSweep_L | TwinSword_Attack20 | 右手左下起双斜斩腾空接落地横扫 |

---

### 1.8 翻滚攻击

| OutputMontageTag | 源动画 | 三元组描述 |
|-----------------|--------|-----------|
| State.Combo.Dbl.RollAttack | TwinSword_Attack21 | 前右起踏步横斩接转身横扫 |
| State.Combo.Dbl.RollAttack_L | TwinSword_Attack22 | 前左起踏步横斩接转身横扫 |

---

### 1.9 非连招动作

以下动画不参与连招表，注入 WeaponAnimSet 其他字段或由外部系统调用。

| 用途 | 源动画文件 | 归属字段 |
|------|-----------|---------|
| 待机姿态 | TwinSword_Idle | `WeaponAnimSet.IdleAdditivePose` |
| 拔刀 | TwinSword_Equip | 外部 PlayAnimMontage |
| 收刀 | TwinSword_Unarm | 外部 PlayAnimMontage |
| 防御起手 | TwinSword_Defense_Start | Defense GA |
| 防御Loop | TwinSword_Defense_Loop | Defense GA |
| 防御收尾 | TwinSword_Defense_End | Defense GA |
| 闪避(后) | TwinSword_Dodge_B | Dodge GA |
| 闪避(后2) | TwinSword_Dodge_B_1 | Dodge GA |
| 闪避(前) | TwinSword_Dodge_F | Dodge GA |
| 闪避(左) | TwinSword_Dodge_L | Dodge GA |
| 闪避(右) | TwinSword_Dodge_R | Dodge GA |
| 滑步(后) | TwinSword_Slide_B | Slide GA |
| 滑步(前) | TwinSword_Slide_F | Slide GA |
| 滑步(左) | TwinSword_Slide_L | Slide GA |
| 滑步(右) | TwinSword_Slide_R | Slide GA |
| 背刺 | TwinSword_Attack_Ambush | 执行系统 |
| 被背刺 | TwinSword_Attack_Ambushed | 执行系统 |
| 背刺失败 | TwinSword_Attack_Ambush_fail | 执行系统 |
| 处决 | TwinSword_Attack_Execution1~3 | 执行系统 |
| 被处决 | TwinSword_Attack_Executed1~3 | 执行系统 |

---

### 1.10 整体连招树

```
待机状态
├─ Light
│   └─ State.Combo.Dbl.Light_1(左上起双斜斩)
│       ├─ Light → State.Combo.Dbl.Light_2(跨两步右上起)
│       │   ├─ Light → State.Combo.Dbl.Light_3(跨两步左上起)
│       │   │   ├─ Light → State.Combo.Dbl.Light_4_End(右上横扫终) ──→ 连段结束
│       │   │   ├─ Heavy → State.Combo.Dbl.Heavy_Slash(蓄力右上双斜斩)
│       │   │   └─ Special → State.Combo.Dbl.Uppercut(上挑腾空)
│       │   │                    └─ [进入空中状态] →
│       │   │                        ├─ Light → Air连段(5段→落地)
│       │   │                        └─ Heavy → 空翻滚(→落地)
│       │   ├─ Heavy → State.Combo.Dbl.Heavy_Thrust_L(左手蓄力刺)
│       │   └─ Special → State.Combo.Dbl.Uppercut(右手上挑腾空)
│       │                    └─ [进入空中状态]
│       ├─ Heavy → State.Combo.Dbl.Heavy_Thrust(右手蓄力刺)
│       └─ Special → State.Combo.Dbl.Uppercut(上挑腾空)
│
├─ Heavy → State.Combo.Dbl.Heavy_Thrust(右手蓄力前刺)[待机起手]
│
└─ Roll → State.Combo.Dbl.RollAttack(踏步横斩)
```

---

### 1.11 动画文件重命名映射

源动画 → 蒙太奇文件名转换规则：`AM_{武器缩写}_{动作名}`

| 当前文件名 | 新文件名(蒙太奇) | OutputMontageTag |
|-----------|----------------|-----------------|
| TwinSword_Attack13 | AM_Dbl_Light1 | State.Combo.Dbl.Light_1 |
| TwinSword_Attack15 | AM_Dbl_Light2 | State.Combo.Dbl.Light_2 |
| TwinSword_Attack16 | AM_Dbl_Light3 | State.Combo.Dbl.Light_3 |
| TwinSword_Attack17 | AM_Dbl_Light4_End | State.Combo.Dbl.Light_4_End |
| TwinSword_Attack25 | AM_Dbl_HeavyThrust | State.Combo.Dbl.Heavy_Thrust |
| TwinSword_Attack26 | AM_Dbl_HeavyThrust_L | State.Combo.Dbl.Heavy_Thrust_L |
| TwinSword_Attack27 | AM_Dbl_HeavySlash | State.Combo.Dbl.Heavy_Slash |
| TwinSword_Attack29 | AM_Dbl_Uppercut | State.Combo.Dbl.Uppercut |
| TwinSword_Attack30 | AM_Dbl_Uppercut_L | State.Combo.Dbl.Uppercut_L |
| TwinSword_Air_Attack_1 | AM_Dbl_Air1 | State.Combo.Dbl.Air_1 |
| TwinSword_Air_Attack_2 | AM_Dbl_Air2 | State.Combo.Dbl.Air_2 |
| TwinSword_Air_Attack_3 | AM_Dbl_Air3 | State.Combo.Dbl.Air_3 |
| TwinSword_Air_Attack_4 | AM_Dbl_Air4 | State.Combo.Dbl.Air_4 |
| TwinSword_Air_Attack_5 | AM_Dbl_Air5_End | State.Combo.Dbl.Air_5_End |
| TwinSword_Air_Attack_6_Start | AM_Dbl_AirSpin_Start | State.Combo.Dbl.Air_Spin_Start |
| TwinSword_Air_Attack_6_Loop | AM_Dbl_AirSpin_Loop | State.Combo.Dbl.Air_Spin_Loop |
| TwinSword_Air_Attack_6_End | AM_Dbl_AirSpin_End | State.Combo.Dbl.Air_Spin_End |
| TwinSword_Air_Attack_6 | AM_Dbl_AirSpin | State.Combo.Dbl.Air_Spin |
| TwinSword_Attack19 | AM_Dbl_RiseSweep | State.Combo.Dbl.Special_RiseSweep |
| TwinSword_Attack21 | AM_Dbl_RollAttack | State.Combo.Dbl.RollAttack |
| TwinSword_Attack32 | AM_Dbl_AirSpin_R | State.Combo.Dbl.Air_Spin_R |
| TwinSword_Attack31 | AM_Dbl_AirSpin_L | State.Combo.Dbl.Air_Spin_L |

---

### 1.12 未使用动画（备选/扩展）

以下动画当前未纳入主力连招，保留作为后续双剑进阶招式或变体的素材：

| 源文件 | 三元组描述 | 潜在用途 |
|--------|-----------|---------|
| TwinSword_Attack14 | 右上起双斜斩 | 备用,Light 2原候选 |
| TwinSword_Attack18 | 左上起双斜斩接转身横扫 | Light 4 镜像版 |
| TwinSword_Attack20 | 右手左下起双斜斩腾空接落地横扫 | Special 镜像版 |
| TwinSword_Attack22 | 前左起踏步横斩接转身横扫 | RollAttack 镜像版 |
| TwinSword_Attack23 | 右上起往复单斜斩 | 单剑派生/蓄力变体 |
| TwinSword_Attack24 | 左起往复单横斩 | 单剑派生/蓄力变体 |
| TwinSword_Attack28 | 蓄力左上起双斜斩 | Heavy_Slash 镜像版 |

---

## 2 其他武器（预留）

> 后续武器连招设计在此扩展。每种武器一个独立章节，结构参考第 1 章双剑模板。

### 预留武器列表

| 武器类型 | 章节 | 状态 |
|---------|------|------|
| Sword（剑） | 2.1 | 待设计 |
| GreatSword（大剑） | 2.2 | 待设计 |
| Dagger（匕首） | 2.3 | 待设计 |
| Spear（矛） | 2.4 | 待设计 |
| Shield（盾牌） | 2.5 | 待设计 |
| Bow（弓） | 2.6 | 待设计 |
| Staff（法杖） | 2.7 | 待设计 |

---

> 文档版本：v1.0
> 最后更新：2026-06-01
> 更新说明：初版，完成双剑（DualBlades）连招设计
