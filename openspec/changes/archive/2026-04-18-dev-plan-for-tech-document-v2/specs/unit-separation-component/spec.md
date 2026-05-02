# Spec Delta: unit-separation-component

## ADDED Requirements

### Requirement: 系统 SHALL 新增 UTsnUnitSeparationComponent 移动单位间分离力

系统 SHALL 提供 `UTsnUnitSeparationComponent`，挂载在每个移动 AI 战斗单位身上，作为 DetourCrowd 群体避障的补充层。

以 `TickInterval = 0.033f`（~30Hz）运行，通过球形 Overlap 查询周围 `ECC_Pawn` 对象，计算分离方向并通过 `AddMovementInput` 注入（与路径跟随输入自然融合，在 `CalcVelocity` 中统一处理）。

该组件**只负责 `Moving↔Moving` 的局部 spacing**。`Moving↔Stance` 的 keep-out 由 `stance-repulsion-subsystem` 独占负责；若附近存在 `Stance` 单位，本组件只能把它们当作约束方向，裁剪朝内分量，不能直接对其产出 separation 力。

**仅对 Moving 状态单位生效**：若本组件所有者处于 Stance 状态，TickComponent 跳过所有逻辑。

#### Scenario: 分离力通过 AddMovementInput 注入不冲突

**Given** 两个移动单位 A、B，A 有 TsnUnitSeparationComponent，A-B 距离 < SeparationRadius  
**When** A 的 TickComponent 执行  
**Then**  
  1. SeparationDir = (A.Loc - B.Loc).GetSafeNormal2D() × LinearFactor  
  2. `MyPawn->AddMovementInput(SeparationDir, SeparationStrength)` 被调用  
  3. 该输入累加到 PendingInputVector，与路径跟随输入一同在 `CalcVelocity` 中处理  
  4. 不直接修改 Velocity，无写入冲突

#### Scenario: Stance 状态时跳过所有逻辑

**Given** 单位当前为 ETsnMobilityStance::Stance  
**When** TickComponent 执行  
**Then** 提前 return，不做 Overlap 查询，不调用 AddMovementInput

#### Scenario: 站桩单位不参与 Moving↔Moving separation 产力

**Given** 单位 A 为 Moving，周围有一个 Moving 单位 B 和一个 Stance 单位 C，且 B、C 都在 SeparationRadius 内  
**When** A 的 TickComponent 执行  
**Then**  
  1. B 可参与 `SeparationDir` 累加  
  2. C 不作为 separation 邻居直接产力  
  3. `Moving↔Stance` 的 keep-out 仍由 `stance-repulsion-subsystem` 负责

#### Scenario: 邻近站桩单位时移除朝内分量

**Given** 单位 A 计算出的 `SeparationCandidate` 与某 Stance 单位的 `InwardToStance` 点积 > 0  
**When** A 在输出 `AddMovementInput` 前执行约束裁剪  
**Then**  
  1. 从 `SeparationCandidate` 中移除 `Project(SeparationCandidate, InwardToStance)`  
  2. 裁剪后的输出向量满足 `Dot(Output, InwardToStance) <= 0`  
  3. 最终只保留切向或远离该 Stance 单位的分量

#### Scenario: 多个 Stance 单位时累加方向后单次裁剪

**Given** 单位 A 附近有两个 Stance 单位 C1、C2，分别位于 A 的东侧和北侧  
**When** A 的 TickComponent 执行约束裁剪  
**Then**  
  1. 收集 C1 的 `InwardToStance1`（东向）和 C2 的 `InwardToStance2`（北向）  
  2. 计算综合约束方向 `CombinedInward = Normalize(InwardToStance1 + InwardToStance2)`（东北向）  
  3. 对最终 SeparationVector 做**一次**投影裁剪，移除朝 CombinedInward 的分量  
  4. 裁剪结果与遍历顺序无关，避免逐个裁剪导致的过度衰减

#### Scenario: 线性衰减——距离越近推力越大

**Given** SeparationRadius = 120，单位 B 在 A 的 50 cm 处（Factor = 1 - 50/120 ≈ 0.583）  
  单位 C 在 A 的 100 cm 处（Factor = 1 - 100/120 ≈ 0.167）  
**When** TickComponent 计算  
**Then** B 产生的分离向量强度 > C 产生的分离向量强度（近者推力更强）

#### Scenario: 调试可视化绘制（仅 Debug/Development）

**Given** `bDrawDebugSeparation == true` 且构建配置包含 `ENABLE_DRAW_DEBUG`  
**When** TickComponent 执行  
**Then** 绘制分离力方向（蓝色箭头）和 Stance 约束裁剪后的最终方向（绿色箭头），SeparationRadius 以半透明球可视化

#### Scenario: OnOwnerReleased 对象池回收清理

**Given** 宿主项目使用对象池管理单位，回收时调用 `OnOwnerReleased()`  
**When** `OnOwnerReleased()` 执行  
**Then**  
  1. 停止 TickComponent（`SetComponentTickEnabled(false)`）  
  2. 下次重新激活时通过 `SetComponentTickEnabled(true)` 恢复

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/Components/TsnUnitSeparationComponent.h` | ADDED |
| `Private/Components/TsnUnitSeparationComponent.cpp` | ADDED |
