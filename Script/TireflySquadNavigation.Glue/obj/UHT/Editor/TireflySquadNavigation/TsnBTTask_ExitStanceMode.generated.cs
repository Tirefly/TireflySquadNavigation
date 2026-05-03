using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 将单位从站姿模式恢复为移动状态，在攻击结束或切换目标时执行
/// </summary>
[UClass, GeneratedType("TsnBTTask_ExitStanceMode", "UnrealSharp.TireflySquadNavigation.TsnBTTask_ExitStanceMode")]
public partial class UTsnBTTask_ExitStanceMode : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTTask_ExitStanceMode).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTTask_ExitStanceMode");
    
}