using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UStruct, GeneratedType("TsnStateTreeTask_MoveToEngagementSlotInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeTask_MoveToEngagementSlotInstanceData")]
public partial struct FTsnStateTreeTask_MoveToEngagementSlotInstanceData : MarshalledStruct<FTsnStateTreeTask_MoveToEngagementSlotInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeTask_MoveToEngagementSlotInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeTask_MoveToEngagementSlotInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定 AIController，由 AI Schema 自动注入
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    // TargetActor
    
    static int TargetActor_Offset;
    /// <summary>
    /// 攻击目标
    /// </summary>
    public UnrealSharp.Engine.AActor TargetActor;
    
    // AcceptanceRadius
    
    static int AcceptanceRadius_Offset;
    /// <summary>
    /// 槽位到达判定容差（cm），距槽位世界快照 ≤ 此值视为到达
    /// </summary>
    public float AcceptanceRadius;
    
    // RePathCheckInterval
    
    static int RePathCheckInterval_Offset;
    /// <summary>
    /// 检查重新寻路的间隔（秒）
    /// </summary>
    public float RePathCheckInterval;
    
    // RePathDistanceThreshold
    
    static int RePathDistanceThreshold_Offset;
    /// <summary>
    /// 目标移动超过此距离才触发重新寻路
    /// </summary>
    public float RePathDistanceThreshold;
    
    // MaxApproachTime
    
    static int MaxApproachTime_Offset;
    /// <summary>
    /// 预占位接近的最大允许时间（秒），0 = 不限时
    /// </summary>
    public float MaxApproachTime;
    
    // HeldSlotComponent
    
    static int HeldSlotComponent_Offset;
    /// <summary>
    /// 运行时缓存：当前持有的槽位组件（用于失败/中断退出时释放）
    /// </summary>
    public TWeakObjectPtr<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent> HeldSlotComponent;
    
    // CachedSlotPosition
    
    static int CachedSlotPosition_Offset;
    public UnrealSharp.CoreUObject.FVector CachedSlotPosition;
    
    // CachedTargetLocation
    
    static int CachedTargetLocation_Offset;
    public UnrealSharp.CoreUObject.FVector CachedTargetLocation;
    
    // CachedEngagementRange
    
    static int CachedEngagementRange_Offset;
    public float CachedEngagementRange;
    
    // ElapsedApproachTime
    
    static int ElapsedApproachTime_Offset;
    public float ElapsedApproachTime;
    
    // TimeSinceLastRePathCheck
    
    static int TimeSinceLastRePathCheck_Offset;
    public float TimeSinceLastRePathCheck;
    
    // SlotAcquired
    
    static int bSlotAcquired_Offset;
    /// <summary>
    /// 任务是否已申请到槽位（决定 ExitState 时是否需要释放）
    /// </summary>
    public bool SlotAcquired;
    
    // ExitedWithSuccess
    
    static int bExitedWithSuccess_Offset;
    /// <summary>
    /// 本次任务是否以成功结束（成功保留槽位，失败/中断释放槽位）
    /// </summary>
    public bool ExitedWithSuccess;
    
    public FTsnStateTreeTask_MoveToEngagementSlotInstanceData(UnrealSharp.AIModule.AAIController AIController, UnrealSharp.Engine.AActor TargetActor, float AcceptanceRadius, float RePathCheckInterval, float RePathDistanceThreshold, float MaxApproachTime, TWeakObjectPtr<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent> HeldSlotComponent, UnrealSharp.CoreUObject.FVector CachedSlotPosition, UnrealSharp.CoreUObject.FVector CachedTargetLocation, float CachedEngagementRange, float ElapsedApproachTime, float TimeSinceLastRePathCheck, bool SlotAcquired, bool ExitedWithSuccess)
    {
        this.AIController = AIController;
        this.TargetActor = TargetActor;
        this.AcceptanceRadius = AcceptanceRadius;
        this.RePathCheckInterval = RePathCheckInterval;
        this.RePathDistanceThreshold = RePathDistanceThreshold;
        this.MaxApproachTime = MaxApproachTime;
        this.HeldSlotComponent = HeldSlotComponent;
        this.CachedSlotPosition = CachedSlotPosition;
        this.CachedTargetLocation = CachedTargetLocation;
        this.CachedEngagementRange = CachedEngagementRange;
        this.ElapsedApproachTime = ElapsedApproachTime;
        this.TimeSinceLastRePathCheck = TimeSinceLastRePathCheck;
        this.SlotAcquired = SlotAcquired;
        this.ExitedWithSuccess = ExitedWithSuccess;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeTask_MoveToEngagementSlotInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr TargetActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActor");
        TargetActor_Offset = CallGetPropertyOffset(TargetActor_NativeProperty);
        IntPtr AcceptanceRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AcceptanceRadius");
        AcceptanceRadius_Offset = CallGetPropertyOffset(AcceptanceRadius_NativeProperty);
        IntPtr RePathCheckInterval_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "RePathCheckInterval");
        RePathCheckInterval_Offset = CallGetPropertyOffset(RePathCheckInterval_NativeProperty);
        IntPtr RePathDistanceThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "RePathDistanceThreshold");
        RePathDistanceThreshold_Offset = CallGetPropertyOffset(RePathDistanceThreshold_NativeProperty);
        IntPtr MaxApproachTime_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "MaxApproachTime");
        MaxApproachTime_Offset = CallGetPropertyOffset(MaxApproachTime_NativeProperty);
        IntPtr HeldSlotComponent_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "HeldSlotComponent");
        HeldSlotComponent_Offset = CallGetPropertyOffset(HeldSlotComponent_NativeProperty);
        IntPtr CachedSlotPosition_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CachedSlotPosition");
        CachedSlotPosition_Offset = CallGetPropertyOffset(CachedSlotPosition_NativeProperty);
        IntPtr CachedTargetLocation_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CachedTargetLocation");
        CachedTargetLocation_Offset = CallGetPropertyOffset(CachedTargetLocation_NativeProperty);
        IntPtr CachedEngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CachedEngagementRange");
        CachedEngagementRange_Offset = CallGetPropertyOffset(CachedEngagementRange_NativeProperty);
        IntPtr ElapsedApproachTime_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ElapsedApproachTime");
        ElapsedApproachTime_Offset = CallGetPropertyOffset(ElapsedApproachTime_NativeProperty);
        IntPtr TimeSinceLastRePathCheck_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TimeSinceLastRePathCheck");
        TimeSinceLastRePathCheck_Offset = CallGetPropertyOffset(TimeSinceLastRePathCheck_NativeProperty);
        IntPtr bSlotAcquired_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bSlotAcquired");
        bSlotAcquired_Offset = CallGetPropertyOffset(bSlotAcquired_NativeProperty);
        IntPtr bExitedWithSuccess_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bExitedWithSuccess");
        bExitedWithSuccess_Offset = CallGetPropertyOffset(bExitedWithSuccess_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeTask_MoveToEngagementSlotInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            TargetActor = ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(InNativeStruct + TargetActor_Offset, 0);
            AcceptanceRadius = BlittableMarshaller<float>.FromNative(InNativeStruct + AcceptanceRadius_Offset, 0);
            RePathCheckInterval = BlittableMarshaller<float>.FromNative(InNativeStruct + RePathCheckInterval_Offset, 0);
            RePathDistanceThreshold = BlittableMarshaller<float>.FromNative(InNativeStruct + RePathDistanceThreshold_Offset, 0);
            MaxApproachTime = BlittableMarshaller<float>.FromNative(InNativeStruct + MaxApproachTime_Offset, 0);
            HeldSlotComponent = BlittableMarshaller<TWeakObjectPtr<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent>>.FromNative(InNativeStruct + HeldSlotComponent_Offset, 0);
            CachedSlotPosition = BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.FromNative(InNativeStruct + CachedSlotPosition_Offset, 0);
            CachedTargetLocation = BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.FromNative(InNativeStruct + CachedTargetLocation_Offset, 0);
            CachedEngagementRange = BlittableMarshaller<float>.FromNative(InNativeStruct + CachedEngagementRange_Offset, 0);
            ElapsedApproachTime = BlittableMarshaller<float>.FromNative(InNativeStruct + ElapsedApproachTime_Offset, 0);
            TimeSinceLastRePathCheck = BlittableMarshaller<float>.FromNative(InNativeStruct + TimeSinceLastRePathCheck_Offset, 0);
            SlotAcquired = BoolMarshaller.FromNative(InNativeStruct + bSlotAcquired_Offset, 0);
            ExitedWithSuccess = BoolMarshaller.FromNative(InNativeStruct + bExitedWithSuccess_Offset, 0);
        }
    }
    
    public static FTsnStateTreeTask_MoveToEngagementSlotInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeTask_MoveToEngagementSlotInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(buffer + TargetActor_Offset, 0, TargetActor);
            BlittableMarshaller<float>.ToNative(buffer + AcceptanceRadius_Offset, 0, AcceptanceRadius);
            BlittableMarshaller<float>.ToNative(buffer + RePathCheckInterval_Offset, 0, RePathCheckInterval);
            BlittableMarshaller<float>.ToNative(buffer + RePathDistanceThreshold_Offset, 0, RePathDistanceThreshold);
            BlittableMarshaller<float>.ToNative(buffer + MaxApproachTime_Offset, 0, MaxApproachTime);
            BlittableMarshaller<TWeakObjectPtr<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent>>.ToNative(buffer + HeldSlotComponent_Offset, 0, HeldSlotComponent);
            BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.ToNative(buffer + CachedSlotPosition_Offset, 0, CachedSlotPosition);
            BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.ToNative(buffer + CachedTargetLocation_Offset, 0, CachedTargetLocation);
            BlittableMarshaller<float>.ToNative(buffer + CachedEngagementRange_Offset, 0, CachedEngagementRange);
            BlittableMarshaller<float>.ToNative(buffer + ElapsedApproachTime_Offset, 0, ElapsedApproachTime);
            BlittableMarshaller<float>.ToNative(buffer + TimeSinceLastRePathCheck_Offset, 0, TimeSinceLastRePathCheck);
            BoolMarshaller.ToNative(buffer + bSlotAcquired_Offset, 0, SlotAcquired);
            BoolMarshaller.ToNative(buffer + bExitedWithSuccess_Offset, 0, ExitedWithSuccess);
        }
    }
}

public static class FTsnStateTreeTask_MoveToEngagementSlotInstanceDataMarshaller
{
    public static FTsnStateTreeTask_MoveToEngagementSlotInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeTask_MoveToEngagementSlotInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeTask_MoveToEngagementSlotInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeTask_MoveToEngagementSlotInstanceData.NativeDataSize;
    }
}