using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 任务实例数据（每个 StateTree 资产实例独立持有）。
/// 
/// Context 字段由 `UStateTreeAIComponentSchema` 自动提供，
/// 设计师只需绑定 TargetActor 与可选的 PreEngagementRadiusMultiplier。
/// </summary>
[UStruct, GeneratedType("TsnStateTreeTask_ChaseEngagementTargetInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeTask_ChaseEngagementTargetInstanceData")]
public partial struct FTsnStateTreeTask_ChaseEngagementTargetInstanceData : MarshalledStruct<FTsnStateTreeTask_ChaseEngagementTargetInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeTask_ChaseEngagementTargetInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeTask_ChaseEngagementTargetInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定 AIController，由 AI Schema 自动注入
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    // TargetActor
    
    static int TargetActor_Offset;
    /// <summary>
    /// 追击目标
    /// </summary>
    public UnrealSharp.Engine.AActor TargetActor;
    
    // PreEngagementRadiusMultiplier
    
    static int PreEngagementRadiusMultiplier_Offset;
    /// <summary>
    /// 预战斗距离倍数（相对于 AttackRange）；进入此半径后任务 Succeeded。
    /// 与现有 BT 节点语义保持一致。
    /// </summary>
    public float PreEngagementRadiusMultiplier;
    
    // CachedEngagementRange
    
    static int CachedEngagementRange_Offset;
    /// <summary>
    /// 运行时缓存：Pawn 的攻击距离（cm）
    /// </summary>
    public float CachedEngagementRange;
    
    public FTsnStateTreeTask_ChaseEngagementTargetInstanceData(UnrealSharp.AIModule.AAIController AIController, UnrealSharp.Engine.AActor TargetActor, float PreEngagementRadiusMultiplier, float CachedEngagementRange)
    {
        this.AIController = AIController;
        this.TargetActor = TargetActor;
        this.PreEngagementRadiusMultiplier = PreEngagementRadiusMultiplier;
        this.CachedEngagementRange = CachedEngagementRange;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeTask_ChaseEngagementTargetInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr TargetActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActor");
        TargetActor_Offset = CallGetPropertyOffset(TargetActor_NativeProperty);
        IntPtr PreEngagementRadiusMultiplier_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PreEngagementRadiusMultiplier");
        PreEngagementRadiusMultiplier_Offset = CallGetPropertyOffset(PreEngagementRadiusMultiplier_NativeProperty);
        IntPtr CachedEngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CachedEngagementRange");
        CachedEngagementRange_Offset = CallGetPropertyOffset(CachedEngagementRange_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeTask_ChaseEngagementTargetInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            TargetActor = ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(InNativeStruct + TargetActor_Offset, 0);
            PreEngagementRadiusMultiplier = BlittableMarshaller<float>.FromNative(InNativeStruct + PreEngagementRadiusMultiplier_Offset, 0);
            CachedEngagementRange = BlittableMarshaller<float>.FromNative(InNativeStruct + CachedEngagementRange_Offset, 0);
        }
    }
    
    public static FTsnStateTreeTask_ChaseEngagementTargetInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeTask_ChaseEngagementTargetInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(buffer + TargetActor_Offset, 0, TargetActor);
            BlittableMarshaller<float>.ToNative(buffer + PreEngagementRadiusMultiplier_Offset, 0, PreEngagementRadiusMultiplier);
            BlittableMarshaller<float>.ToNative(buffer + CachedEngagementRange_Offset, 0, CachedEngagementRange);
        }
    }
}

public static class FTsnStateTreeTask_ChaseEngagementTargetInstanceDataMarshaller
{
    public static FTsnStateTreeTask_ChaseEngagementTargetInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeTask_ChaseEngagementTargetInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeTask_ChaseEngagementTargetInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeTask_ChaseEngagementTargetInstanceData.NativeDataSize;
    }
}