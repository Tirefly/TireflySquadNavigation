using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UStruct, GeneratedType("TsnStateTreeEvaluator_CombatContextInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeEvaluator_CombatContextInstanceData")]
public partial struct FTsnStateTreeEvaluator_CombatContextInstanceData : MarshalledStruct<FTsnStateTreeEvaluator_CombatContextInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeEvaluator_CombatContextInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeEvaluator_CombatContextInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定 AIController
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    // TargetActor
    
    static int TargetActor_Offset;
    /// <summary>
    /// 当前目标
    /// </summary>
    public UnrealSharp.Engine.AActor TargetActor;
    
    // PreEngagementRadiusMultiplier
    
    static int PreEngagementRadiusMultiplier_Offset;
    /// <summary>
    /// 预战斗距离倍数（相对于 AttackRange）
    /// </summary>
    public float PreEngagementRadiusMultiplier;
    
    // HasValidTarget
    
    static int bHasValidTarget_Offset;
    /// <summary>
    /// 输出：目标是否有效（包含 Pawn / 接口校验）
    /// </summary>
    public bool HasValidTarget;
    
    // DistanceToTarget2D
    
    static int DistanceToTarget2D_Offset;
    /// <summary>
    /// 输出：当前 2D 距离（cm）；目标无效时为 0
    /// </summary>
    public float DistanceToTarget2D;
    
    // EngagementRange
    
    static int EngagementRange_Offset;
    /// <summary>
    /// 输出：Pawn 的攻击距离（cm）；目标无效时为 0
    /// </summary>
    public float EngagementRange;
    
    // PreEngagementRadius
    
    static int PreEngagementRadius_Offset;
    /// <summary>
    /// 输出：预战斗距离（cm）；目标无效时为 0
    /// </summary>
    public float PreEngagementRadius;
    
    // IsInEngagementRange
    
    static int bIsInEngagementRange_Offset;
    /// <summary>
    /// 输出：是否已进入攻击距离
    /// </summary>
    public bool IsInEngagementRange;
    
    // IsInPreEngagementRange
    
    static int bIsInPreEngagementRange_Offset;
    /// <summary>
    /// 输出：是否已进入预战斗距离
    /// </summary>
    public bool IsInPreEngagementRange;
    
    public FTsnStateTreeEvaluator_CombatContextInstanceData(UnrealSharp.AIModule.AAIController AIController, UnrealSharp.Engine.AActor TargetActor, float PreEngagementRadiusMultiplier, bool HasValidTarget, float DistanceToTarget2D, float EngagementRange, float PreEngagementRadius, bool IsInEngagementRange, bool IsInPreEngagementRange)
    {
        this.AIController = AIController;
        this.TargetActor = TargetActor;
        this.PreEngagementRadiusMultiplier = PreEngagementRadiusMultiplier;
        this.HasValidTarget = HasValidTarget;
        this.DistanceToTarget2D = DistanceToTarget2D;
        this.EngagementRange = EngagementRange;
        this.PreEngagementRadius = PreEngagementRadius;
        this.IsInEngagementRange = IsInEngagementRange;
        this.IsInPreEngagementRange = IsInPreEngagementRange;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeEvaluator_CombatContextInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr TargetActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActor");
        TargetActor_Offset = CallGetPropertyOffset(TargetActor_NativeProperty);
        IntPtr PreEngagementRadiusMultiplier_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PreEngagementRadiusMultiplier");
        PreEngagementRadiusMultiplier_Offset = CallGetPropertyOffset(PreEngagementRadiusMultiplier_NativeProperty);
        IntPtr bHasValidTarget_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bHasValidTarget");
        bHasValidTarget_Offset = CallGetPropertyOffset(bHasValidTarget_NativeProperty);
        IntPtr DistanceToTarget2D_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DistanceToTarget2D");
        DistanceToTarget2D_Offset = CallGetPropertyOffset(DistanceToTarget2D_NativeProperty);
        IntPtr EngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EngagementRange");
        EngagementRange_Offset = CallGetPropertyOffset(EngagementRange_NativeProperty);
        IntPtr PreEngagementRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PreEngagementRadius");
        PreEngagementRadius_Offset = CallGetPropertyOffset(PreEngagementRadius_NativeProperty);
        IntPtr bIsInEngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bIsInEngagementRange");
        bIsInEngagementRange_Offset = CallGetPropertyOffset(bIsInEngagementRange_NativeProperty);
        IntPtr bIsInPreEngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bIsInPreEngagementRange");
        bIsInPreEngagementRange_Offset = CallGetPropertyOffset(bIsInPreEngagementRange_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeEvaluator_CombatContextInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            TargetActor = ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(InNativeStruct + TargetActor_Offset, 0);
            PreEngagementRadiusMultiplier = BlittableMarshaller<float>.FromNative(InNativeStruct + PreEngagementRadiusMultiplier_Offset, 0);
            HasValidTarget = BoolMarshaller.FromNative(InNativeStruct + bHasValidTarget_Offset, 0);
            DistanceToTarget2D = BlittableMarshaller<float>.FromNative(InNativeStruct + DistanceToTarget2D_Offset, 0);
            EngagementRange = BlittableMarshaller<float>.FromNative(InNativeStruct + EngagementRange_Offset, 0);
            PreEngagementRadius = BlittableMarshaller<float>.FromNative(InNativeStruct + PreEngagementRadius_Offset, 0);
            IsInEngagementRange = BoolMarshaller.FromNative(InNativeStruct + bIsInEngagementRange_Offset, 0);
            IsInPreEngagementRange = BoolMarshaller.FromNative(InNativeStruct + bIsInPreEngagementRange_Offset, 0);
        }
    }
    
    public static FTsnStateTreeEvaluator_CombatContextInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeEvaluator_CombatContextInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(buffer + TargetActor_Offset, 0, TargetActor);
            BlittableMarshaller<float>.ToNative(buffer + PreEngagementRadiusMultiplier_Offset, 0, PreEngagementRadiusMultiplier);
            BoolMarshaller.ToNative(buffer + bHasValidTarget_Offset, 0, HasValidTarget);
            BlittableMarshaller<float>.ToNative(buffer + DistanceToTarget2D_Offset, 0, DistanceToTarget2D);
            BlittableMarshaller<float>.ToNative(buffer + EngagementRange_Offset, 0, EngagementRange);
            BlittableMarshaller<float>.ToNative(buffer + PreEngagementRadius_Offset, 0, PreEngagementRadius);
            BoolMarshaller.ToNative(buffer + bIsInEngagementRange_Offset, 0, IsInEngagementRange);
            BoolMarshaller.ToNative(buffer + bIsInPreEngagementRange_Offset, 0, IsInPreEngagementRange);
        }
    }
}

public static class FTsnStateTreeEvaluator_CombatContextInstanceDataMarshaller
{
    public static FTsnStateTreeEvaluator_CombatContextInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeEvaluator_CombatContextInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeEvaluator_CombatContextInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeEvaluator_CombatContextInstanceData.NativeDataSize;
    }
}