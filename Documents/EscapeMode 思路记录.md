# EscapeMode 思路记录

## 背景

当前 `UTsnTacticalMovementComponent::CalcVelocity()` 仍以 UE 路径跟随计算出的 `Velocity` 为主，
再叠加来自 `UTsnStanceRepulsionSubsystem` 的排斥速度。

当以下情况出现时，说明当前局部路径已经失效：

- `Velocity` 与 `Repulsion` 明显相反
- 单位仍位于一个或多个 Stance 单位的 keep-out 区域内
- 排斥刚把单位推出拥挤区域，下一帧路径速度又把它拉回去

这类问题本质上不是“局部避让不够强”，而是“当前帧的导航意图已经不可信”。

## 目标

- 在局部路径显著错误时，短时间放弃当前路径前进分量
- 优先让单位脱离 Stance keep-out 区域
- 避免“推出去一帧，下一帧又回来”的抖动
- 退出 EscapeMode 后，再恢复正常路径驱动

## 初步策略

### 1. 触发条件

候选触发条件：

- `Dot(PathVelocityDir, RepulsionDir) < -0.2`
- `Repulsion` 强度达到最小阈值，例如 `MaxWalkSpeed * 0.15`
- 单位位于至少一个 `EffectiveNavModifierRadius` 或其外侧小缓冲带内
- 条件连续满足 2 帧以上，避免单帧抖动误触发

### 2. EscapeMode 行为

触发后，短时间内不再信任当前路径前进分量：

- 最终速度以 `Repulsion` 为主
- 允许保留与脱离方向不冲突的切向分量
- 丢弃继续朝 Stance 单位或窄缝内部推进的路径分量
- 不在每一帧都立刻重新寻路

### 3. 退出条件

建议使用滞后退出，而不是单帧立即恢复：

- 离开 `EffectiveNavModifierRadius + ReleaseMargin`
- 或保持 `0.2s ~ 0.35s` 的最短逃逸持续时间
- 满足退出条件若干帧后再恢复正常路径速度

### 4. Repath 策略

- 进入 EscapeMode 时先不急于 Repath
- 先用局部逃逸把单位推出错误区域
- 退出 EscapeMode 后，如果目标仍在前方且当前路径意图仍穿过障碍带，再触发一次 Repath

## 调试观察点

当前已可配合以下调试信息观察现象：

- `bDrawDebugRepulsion`
  - 青色箭头：路径速度
  - 橙色箭头：排斥速度
  - 绿色箭头：最终速度
- `bDrawDebugObstacle` 或控制台变量 `tsn.debug.DrawStanceObstacle 1`
  - 红圈：`ObstacleRadius`
  - 蓝色圆柱/青圈：实际 `NavModifier` 半径
  - 黄圈：`RepulsionRadius`

## 当前状态

- 本文档只记录 EscapeMode 方案
- 当前代码尚未实现 EscapeMode 状态机
- 现阶段仍保持“路径速度 + 排斥速度叠加，再由 keep-out 防护裁剪”的实现