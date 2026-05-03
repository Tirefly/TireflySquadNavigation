using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 站姿单位高代价导航区域。
/// 
/// 在 ETsnNavModifierMode::HighCost 策略下使用。
/// DefaultCost = 10.0：寻路算法强烈倾向绕行，但不完全封死。
/// 棋盘格地图下推荐使用 ETsnNavModifierMode::Impassable（NavArea_Null），
/// 彻底封路以确保绕行。
/// </summary>
[UClass, GeneratedType("TsnNavArea_StanceUnit", "UnrealSharp.TireflySquadNavigation.TsnNavArea_StanceUnit")]
public partial class UTsnNavArea_StanceUnit : UnrealSharp.NavigationSystem.UNavArea
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnNavArea_StanceUnit).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnNavArea_StanceUnit");
    
}