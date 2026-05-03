using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 遍历场景中所有敌方阵营的棋子 / 靶标，选择最近的存活目标。
/// 
/// 搜索条件：
/// - ATsnTestChessPiece：TeamID 与自身不同
/// - ATsnTestTargetDummy：始终视为有效目标
/// 
/// 同步完成，不使用 InProgress。
/// </summary>
[UClass, GeneratedType("TsnTestBTTask_SelectTarget", "UnrealSharp.TireflySquadNavigationTest.TsnTestBTTask_SelectTarget")]
public partial class UTsnTestBTTask_SelectTarget : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnTestBTTask_SelectTarget).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestBTTask_SelectTarget");
    static UTsnTestBTTask_SelectTarget()
    {
        IntPtr TargetActorKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActorKey");
        TargetActorKey_Offset = CallGetPropertyOffset(TargetActorKey_NativeProperty);
        IntPtr HasTargetKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "HasTargetKey");
        HasTargetKey_Offset = CallGetPropertyOffset(HasTargetKey_NativeProperty);
        IntPtr SearchRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SearchRadius");
        SearchRadius_Offset = CallGetPropertyOffset(SearchRadius_NativeProperty);
    }
    
    static int TargetActorKey_Offset;
    
    /// <summary>
    /// 写入选中目标 Actor 的 Blackboard Key
    /// </summary>
    public UnrealSharp.AIModule.FBlackboardKeySelector TargetActorKey
    {
        get
        {
            return StructMarshaller<UnrealSharp.AIModule.FBlackboardKeySelector>.FromNative(NativeObject + TargetActorKey_Offset, 0);
        }
        set
        {
            StructMarshaller<UnrealSharp.AIModule.FBlackboardKeySelector>.ToNative(NativeObject + TargetActorKey_Offset, 0, value);
        }
    }
    
    
    static int HasTargetKey_Offset;
    
    /// <summary>
    /// 写入是否找到目标的 Blackboard Key
    /// </summary>
    public UnrealSharp.AIModule.FBlackboardKeySelector HasTargetKey
    {
        get
        {
            return StructMarshaller<UnrealSharp.AIModule.FBlackboardKeySelector>.FromNative(NativeObject + HasTargetKey_Offset, 0);
        }
        set
        {
            StructMarshaller<UnrealSharp.AIModule.FBlackboardKeySelector>.ToNative(NativeObject + HasTargetKey_Offset, 0, value);
        }
    }
    
    
    static int SearchRadius_Offset;
    
    /// <summary>
    /// 最大搜索半径 (cm)
    /// </summary>
    public float SearchRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + SearchRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + SearchRadius_Offset, 0, value);
        }
    }
    
    
}