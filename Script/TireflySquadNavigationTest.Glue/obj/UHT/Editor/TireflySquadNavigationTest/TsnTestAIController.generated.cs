using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 测试用 AI 控制器。
/// 
/// 在构造函数中将默认 PathFollowingComponent 替换为 UCrowdFollowingComponent，
/// 以启用 DetourCrowd 群体避障。
/// CMC 内置 RVO Avoidance 由 UTsnTacticalMovementComponent 构造函数统一关闭，
/// AIController 不再重复承担该职责。
/// </summary>
[UClass, GeneratedType("TsnTestAIController", "UnrealSharp.TireflySquadNavigationTest.TsnTestAIController")]
public partial class ATsnTestAIController : UnrealSharp.AIModule.AAIController
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(ATsnTestAIController).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestAIController");
    
}