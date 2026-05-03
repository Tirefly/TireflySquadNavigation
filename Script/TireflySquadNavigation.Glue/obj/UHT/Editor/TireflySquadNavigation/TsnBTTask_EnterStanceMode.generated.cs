using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 将单位切换为站姿模式，在 MoveToEngagementSlot 之后执行
/// </summary>
[UClass, GeneratedType("TsnBTTask_EnterStanceMode", "UnrealSharp.TireflySquadNavigation.TsnBTTask_EnterStanceMode")]
public partial class UTsnBTTask_EnterStanceMode : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTTask_EnterStanceMode).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTTask_EnterStanceMode");
    
}