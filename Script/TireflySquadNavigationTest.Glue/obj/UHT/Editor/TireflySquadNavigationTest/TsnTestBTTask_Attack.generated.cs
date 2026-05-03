using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 模拟攻击过程，等待指定时长后返回 Succeeded。
/// 
/// 异步执行：ExecuteTask 返回 InProgress，
/// TickTask 中倒计时完成后 FinishLatentTask(Succeeded)。
/// 
/// 使用 bCreateNodeInstance = true，运行时状态存为成员变量。
/// </summary>
[UClass, GeneratedType("TsnTestBTTask_Attack", "UnrealSharp.TireflySquadNavigationTest.TsnTestBTTask_Attack")]
public partial class UTsnTestBTTask_Attack : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnTestBTTask_Attack).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestBTTask_Attack");
    static UTsnTestBTTask_Attack()
    {
        IntPtr AttackDuration_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AttackDuration");
        AttackDuration_Offset = CallGetPropertyOffset(AttackDuration_NativeProperty);
    }
    
    static int AttackDuration_Offset;
    
    /// <summary>
    /// 攻击持续时长（秒）
    /// </summary>
    public float AttackDuration
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + AttackDuration_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + AttackDuration_Offset, 0, value);
        }
    }
    
    
}