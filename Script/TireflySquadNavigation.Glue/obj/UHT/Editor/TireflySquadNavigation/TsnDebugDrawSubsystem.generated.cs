using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// TSN 统一导航调试绘制子系统。
/// 
/// 负责在运行时以命令行控制的方式，为实现了 ITsnTacticalUnit 的单位
/// 绘制多层半径 overlay。该子系统只读取现有运行时状态，
/// 不会改变槽位认领、站姿状态或移动请求。
/// </summary>
[UClass, GeneratedType("TsnDebugDrawSubsystem", "UnrealSharp.TireflySquadNavigation.TsnDebugDrawSubsystem")]
public partial class UTsnDebugDrawSubsystem : UnrealSharp.Engine.UTickableWorldSubsystem
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnDebugDrawSubsystem).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnDebugDrawSubsystem");
    
}