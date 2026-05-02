# Spec Delta: stance-repulsion-subsystem

## ADDED Requirements

### Requirement: 系统 SHALL 新增 UTsnStanceRepulsionSubsystem 双阶段排斥力世界子系统

系统 SHALL 提供 `UTsnStanceRepulsionSubsystem`，继承 `UTickableWorldSubsystem`，自动随关卡创建，无需手动实例化。

每帧遍历已注册的 `MovingUnits` 列表（由 `TsnTacticalMovementComponent` 的 BeginPlay/EndPlay 注册/注销），对靠近站桩单位的**移动中单位**施加双阶段排斥力，通过 `TsnTacticalMovementComponent::SetRepulsionVelocity` 合法注入。遍历范围收窄为 O(战术单位数)，避免与场景中非战斗 Character 误交互。

该子系统在帧 N **只负责生产并写入**排斥速度增量；真正的消费由 `tactical-movement-component` 固定在帧 N+1 的 `CalcVelocity`（`Super` 之后）中完成，不依赖 Tick 先后。

该子系统是 **`Moving↔Stance` keep-out / anti-penetration 的唯一权威来源**。`unit-separation-component` 只能处理 `Moving↔Moving` spacing，不得替代本子系统对站桩单位直接产力。

**关联能力**：`tactical-movement-component`（注入接口 + 移动单位注册/注销 + Penetration Guard 查询），`stance-obstacle-component`（站桩单位注册/注销 + 读 MobilityStance）

**可配置属性**：
- `OuterRepulsionRatio`（默认 `0.1f`，`UPROPERTY(EditAnywhere, Category = "Repulsion", meta = (ClampMin = "0.0", ClampMax = "0.5"))`）：外层排斥力阶段系数，控制外层排斥力相对于 `RepulsionStrength` 的比例。宿主项目可调整此值以平衡外层轻推强度与路径跟随干扰。
- `TangentBlendRatio`（默认 `0.7f`，`UPROPERTY(EditAnywhere, Category = "Repulsion", meta = (ClampMin = "0.0", ClampMax = "1.0"))`）：内层方向混合中切向引导的比例。`1 - TangentBlendRatio` 为径向排斥比例。默认 0.7 表示 70% 切线引导 + 30% 径向排斥。

#### Scenario: 只对移动中的单位施加排斥力

**Given** 场景中有单位 A（Moving）和单位 B（Stance）  
  A 和 B 均在某站桩障碍物 C 的 RepulsionRadius 内  
**When** Subsystem::Tick 执行  
**Then** 对 A 施加排斥力（有 TsnStanceObstacleComponent + Moving）；对 B **不施加**（B 是 Stance 单位）

#### Scenario: 与 UnitSeparation 并存时仍由本子系统主导 Moving↔Stance keep-out

**Given** 移动单位 A 附近同时存在 Moving 单位 B 与 Stance 单位 C  
**When** Subsystem::Tick 执行  
**Then** A 相对于 C 的 keep-out 向量仍只由 C 的 ObstacleRadius / RepulsionRadius / RepulsionStrength 计算得到，不依赖 `unit-separation-component` 的产力结果

#### Scenario: 当前帧 Tick 只写入延迟排斥力，消费发生在下一帧

**Given** Subsystem 在帧 N 计算出某移动单位 A 的 `TotalRepulsion`  
**When** 帧 N 的 `Tick` 调用 `SetRepulsionVelocity(TotalRepulsion * DeltaTime)`  
**Then**  
  1. 该调用只负责把排斥速度写入 `tactical-movement-component` 的延迟缓冲  
  2. 帧 N 不要求在同一次 `Tick` 内立即生效  
  3. 真正的排斥力消费固定发生在帧 N+1 的 `CalcVelocity`（`Super` 之后）

#### Scenario: 内层排斥力全力推出穿入区域

**Given** 移动单位与站桩单位距离 = 30 cm（< ObstacleRadius = 60 cm，即内层区域）  
  RepulsionStrength = 800  
**When** 该帧双阶段力度计算  
**Then**  
  - 内层 Penetration = 1 - (30/60) = 0.5  
  - ForceMagnitude = 800 × 0.5² = 200  
  - 方向：`TangentBlendRatio`（默认 0.7）切线引导 + `(1 - TangentBlendRatio)`（默认 0.3）径向排斥

#### Scenario: 外层排斥力按 RepulsionStrength 的 10% 轻推，不干扰路径跟随

**Given** 移动单位与站桩单位距离 = 100 cm（在 ObstacleRadius=60 ~ RepulsionRadius=150 之间，外层）  
  RepulsionStrength = 800  
**When** 该帧双阶段力度计算  
**Then**  
  - OuterRange = 150 - 60 = 90；OuterDist = 100 - 60 = 40  
  - OuterFactor = 1 - (40/90) ≈ 0.556  
  - ForceMagnitude = 800 × OuterRepulsionRatio × 0.556（其中 `OuterRepulsionRatio` 默认为 `0.1f`，现为可配置的 `UPROPERTY`）
  - 若 OuterRepulsionRatio = 0.1，则 ForceMagnitude ≈ 44.4
  - 该系数仅是外层阶段系数；注入 MovementComponent 后，最终速度仍统一受 `MaxWalkSpeed * MaxRepulsionVelocityRatio` 限幅

#### Scenario: 超出 RepulsionRadius 的单位不受影响

**Given** 距离 > RepulsionRadius  
**When** Tick 执行  
**Then** 该组（移动单位，站桩单位）不计算排斥力，循环直接跳过

#### Scenario: 无站桩单位时 Tick 提前退出

**Given** StanceUnits 数组为空  
**When** Tick 执行  
**Then** 跳过遍历逻辑，不做任何 OverlapQuery 或 Actor 遍历

#### Scenario: 多个站桩单位时排斥力内部合并后单次写入

**Given** 移动单位周围有 2 个站桩单位 C1、C2，均在内层区域  
**When** Tick 执行  
**Then** 分别计算 C1、C2 的排斥向量，将两者累加为 TotalRepulsion，再**只调用一次** `SetRepulsionVelocity(TotalRepulsion * DeltaTime)`——不应对每个 Stance 单位单独调用，避免触发 ensureMsgf 警告

#### Scenario: UpdateStanceUnit 刷新已注册条目的参数

**Given** 站桩单位 A 已注册，当前 RepulsionRadius = 150  
**When** `UpdateStanceUnit(A, 200, 1000, 80)` 被调用  
**Then** StanceUnits 数组中 A 的 RepulsionRadius、RepulsionStrength、NavModifierRadius 均更新为新值；后续帧的排斥力计算使用新参数

#### Scenario: UpdateStanceUnit 对未注册单位无副作用

**Given** 单位 B 未注册到 StanceUnits  
**When** `UpdateStanceUnit(B, ...)` 被调用  
**Then** 无任何操作，不 crash

#### Scenario: ExitStanceMode 后排斥力立即停止，NavModifier 延迟关闭

**Given** 单位刚刚调用了 `ExitStanceMode()`，NavModifier 延迟关闭 Timer 尚未到期  
**When** 其他移动单位经过该区域  
**Then** 排斥力**不再产生**（单位已注销），但 NavMesh 路径规划仍会绕开（NavModifier 尚未关闭）。这是设计意图：延迟关闭仅用于平滑过渡路径规划，不作为持续的物理屏障

#### Scenario: 调试可视化绘制（仅 Debug/Development）

**Given** `bDrawDebugRepulsion == true` 且构建配置包含 `ENABLE_DRAW_DEBUG`  
**When** Tick 执行  
**Then** 为每个 Stance 单位绘制内层/外层圆环，为每个受影响的 Moving 单位绘制排斥力方向箭头

#### Scenario: GetStanceUnits 提供 Penetration Guard 查询接口

**Given** `TsnTacticalMovementComponent` 需要在 CalcVelocity 后执行 Stance Penetration Guard  
**When** 调用 `GetStanceUnits()` 查询当前已注册的 Stance 单位列表  
**Then** 返回包含每个 Stance 单位的 Actor 弱引用和参数值的只读引用，开销为 O(1)（返回引用，不拷贝）；调用方通过 `Obs.Unit->GetActorLocation()` 动态读取位置

#### Scenario: RegisterStanceUnit 幂等——重复注册不产生重复条目

**Given** 单位 A 已通过 `RegisterStanceUnit` 注册  
**When** 同一帧或任意后续帧再次调用 `RegisterStanceUnit(A, ...)` （如 Blueprint 调用者连续两次调用 `EnterStanceMode`）  
**Then** 检测到 A 已存在，直接返回；`StanceUnits` 中 A 的条目数量仍为 1，不新增重复条目

#### Scenario: UnregisterStanceUnit 幂等——对未注册单位无副作用

**Given** 单位 B 未注册到 StanceUnits（或已被注销）  
**When** 调用 `UnregisterStanceUnit(B)`  
**Then** `RemoveAll` 找不到匹配条目，正常返回；顺带清理所有失效的弱引用条目；不 crash，不产生任何副作用

#### Scenario: TangentBlendRatio 控制方向混合比例

**Given** `TangentBlendRatio = 0.5`（宿主项目自定义）  
  移动单位穿入站桩单位 ObstacleRadius 内层  
**When** 内层排斥力方向计算  
**Then**  
  1. 切线方向权重 = 0.5，径向方向权重 = 0.5  
  2. `BlendedDir = Normalize(TangentDir * 0.5 + RadialDir * 0.5)`  
  3. 与默认 0.7/0.3 相比，径向排斥更强，单位更快被推出但路径绕行更少

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/Subsystems/TsnStanceRepulsionSubsystem.h` | ADDED |
| `Private/Subsystems/TsnStanceRepulsionSubsystem.cpp` | ADDED |
