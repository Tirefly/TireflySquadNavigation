using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 生成器阵型类型
/// </summary>
[UEnum, GeneratedType("ETsnTestFormation", "UnrealSharp.TireflySquadNavigationTest.ETsnTestFormation")]
public enum ETsnTestFormation : byte
{
    /// <summary>
    /// 横排一字排开
    /// </summary>
    Line = 0,
    /// <summary>
    /// 纵列排开
    /// </summary>
    Column = 1,
    /// <summary>
    /// 方阵（自动计算行列数）
    /// </summary>
    Grid = 2,
    /// <summary>
    /// 圆形环绕 Spawner 中心
    /// </summary>
    Circle = 3,
}