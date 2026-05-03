using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// NavModifier 区域类型策略。
/// 决定站姿单位脚下的区域在 NavMesh 中如何标记。
/// </summary>
[UEnum, GeneratedType("ETsnNavModifierMode", "UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode")]
public enum ETsnNavModifierMode : byte
{
    /// <summary>
    /// 不可通行（推荐：棋盘格、规整开阔地）。
    /// 使用 NavArea_Null，NavMesh 中该区域被完全封禁。
    /// </summary>
    Impassable = 0,
    /// <summary>
    /// 高代价（适合：复杂地形、走廊、室内）。
    /// 使用 TsnNavArea_StanceUnit，寻路算法优先绕行但不强制。
    /// </summary>
    HighCost = 1,
}