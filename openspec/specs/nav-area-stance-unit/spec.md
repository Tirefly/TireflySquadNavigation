# nav-area-stance-unit Specification

## Purpose
TBD - created by archiving change dev-plan-for-tech-document-v2. Update Purpose after archive.
## Requirements
### Requirement: 系统 SHALL 新增 UTsnNavArea_StanceUnit 导航区域类

系统 SHALL 提供 `UTsnNavArea_StanceUnit`，位于 `Public/NavAreas/TsnNavArea_StanceUnit.h`（仅头文件，无 .cpp）。

继承自 `UNavArea`，用于在 `ETsnNavModifierMode::HighCost` 策略下标记站桩单位所在区域。构造函数中设置以下参数：

- `DefaultCost = 10.f`：穿越站桩区域的导航代价倍率，使寻路算法强烈倾向绕行但不强制封路
- `FixedAreaEnteringCost = 5.f`：进入该区域的固定额外代价，进一步提高绕行优先级
- `DrawColor = FColor(255, 128, 0, 255)`：橙色，在导航调试视图中与 NavArea_Null（红色）区分

适用于复杂地形场景（走廊、室内）。

**与 NavArea_Null 的区别**：`NavArea_Null` 完全封禁区域（对应 Impassable 策略）；`UTsnNavArea_StanceUnit` 仅提高通行代价（对应 HighCost 策略），不会造成路径死锁。

#### Scenario: HighCost 模式下单位会绕开站桩区域

**Given** `TsnStanceObstacleComponent.NavModifierMode == HighCost`  
  且 NavMesh 上存在站桩单位占据的区域  
**When** 移动单位请求路径规划  
**Then** 路径算法优先选择不经过 UTsnNavArea_StanceUnit 区域的路径；若无绕行路径则仍可通过（不死锁）

#### Scenario: Impassable 模式下使用 NavArea_Null 而非 StanceUnit

**Given** `TsnStanceObstacleComponent.NavModifierMode == Impassable`  
**When** `ApplyNavAreaClass` 执行  
**Then** `NavModifierComp->SetAreaClass(UNavArea_Null::StaticClass())` 被调用，而非 `UTsnNavArea_StanceUnit::StaticClass()`

---

