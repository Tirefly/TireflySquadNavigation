using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UStruct, GeneratedType("TsnStateTreeEvaluator_TargetMotionInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeEvaluator_TargetMotionInstanceData")]
public partial struct FTsnStateTreeEvaluator_TargetMotionInstanceData : MarshalledStruct<FTsnStateTreeEvaluator_TargetMotionInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeEvaluator_TargetMotionInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeEvaluator_TargetMotionInstanceData");
    // TargetActor
    
    static int TargetActor_Offset;
    public UnrealSharp.Engine.AActor TargetActor;
    
    // SpeedThreshold
    
    static int SpeedThreshold_Offset;
    /// <summary>
    /// 静止判定阈值（cm/s）；与现有 TsnBTDecorator_IsTargetStationary 一致
    /// </summary>
    public float SpeedThreshold;
    
    // TargetSpeed2D
    
    static int TargetSpeed2D_Offset;
    /// <summary>
    /// 输出：目标 2D 速度（cm/s）
    /// </summary>
    public float TargetSpeed2D;
    
    // IsTargetStationary
    
    static int bIsTargetStationary_Offset;
    /// <summary>
    /// 输出：目标是否静止（速度 ≤ 阈值）
    /// </summary>
    public bool IsTargetStationary;
    
    public FTsnStateTreeEvaluator_TargetMotionInstanceData(UnrealSharp.Engine.AActor TargetActor, float SpeedThreshold, float TargetSpeed2D, bool IsTargetStationary)
    {
        this.TargetActor = TargetActor;
        this.SpeedThreshold = SpeedThreshold;
        this.TargetSpeed2D = TargetSpeed2D;
        this.IsTargetStationary = IsTargetStationary;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeEvaluator_TargetMotionInstanceData()
    {
        IntPtr TargetActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActor");
        TargetActor_Offset = CallGetPropertyOffset(TargetActor_NativeProperty);
        IntPtr SpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SpeedThreshold");
        SpeedThreshold_Offset = CallGetPropertyOffset(SpeedThreshold_NativeProperty);
        IntPtr TargetSpeed2D_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetSpeed2D");
        TargetSpeed2D_Offset = CallGetPropertyOffset(TargetSpeed2D_NativeProperty);
        IntPtr bIsTargetStationary_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bIsTargetStationary");
        bIsTargetStationary_Offset = CallGetPropertyOffset(bIsTargetStationary_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeEvaluator_TargetMotionInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            TargetActor = ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(InNativeStruct + TargetActor_Offset, 0);
            SpeedThreshold = BlittableMarshaller<float>.FromNative(InNativeStruct + SpeedThreshold_Offset, 0);
            TargetSpeed2D = BlittableMarshaller<float>.FromNative(InNativeStruct + TargetSpeed2D_Offset, 0);
            IsTargetStationary = BoolMarshaller.FromNative(InNativeStruct + bIsTargetStationary_Offset, 0);
        }
    }
    
    public static FTsnStateTreeEvaluator_TargetMotionInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeEvaluator_TargetMotionInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(buffer + TargetActor_Offset, 0, TargetActor);
            BlittableMarshaller<float>.ToNative(buffer + SpeedThreshold_Offset, 0, SpeedThreshold);
            BlittableMarshaller<float>.ToNative(buffer + TargetSpeed2D_Offset, 0, TargetSpeed2D);
            BoolMarshaller.ToNative(buffer + bIsTargetStationary_Offset, 0, IsTargetStationary);
        }
    }
}

public static class FTsnStateTreeEvaluator_TargetMotionInstanceDataMarshaller
{
    public static FTsnStateTreeEvaluator_TargetMotionInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeEvaluator_TargetMotionInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeEvaluator_TargetMotionInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeEvaluator_TargetMotionInstanceData.NativeDataSize;
    }
}