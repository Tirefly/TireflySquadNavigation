using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 单位移动姿态
/// </summary>
[UEnum, GeneratedType("ETsnMobilityStance", "UnrealSharp.TireflySquadNavigation.ETsnMobilityStance")]
public enum ETsnMobilityStance : byte
{
    /// <summary>
    /// 正在移动，作为正常 Crowd Agent 参与速度协商
    /// </summary>
    Moving = 0,
    /// <summary>
    /// 站桩战斗中，作为动态障碍物
    /// </summary>
    Stance = 1,
}