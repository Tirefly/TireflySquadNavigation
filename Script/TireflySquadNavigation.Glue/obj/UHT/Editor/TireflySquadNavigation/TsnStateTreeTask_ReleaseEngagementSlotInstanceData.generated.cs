using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UStruct, GeneratedType("TsnStateTreeTask_ReleaseEngagementSlotInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeTask_ReleaseEngagementSlotInstanceData")]
public partial struct FTsnStateTreeTask_ReleaseEngagementSlotInstanceData : MarshalledStruct<FTsnStateTreeTask_ReleaseEngagementSlotInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeTask_ReleaseEngagementSlotInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeTask_ReleaseEngagementSlotInstanceData");
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
    
    public FTsnStateTreeTask_ReleaseEngagementSlotInstanceData(UnrealSharp.AIModule.AAIController AIController, UnrealSharp.Engine.AActor TargetActor)
    {
        this.AIController = AIController;
        this.TargetActor = TargetActor;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeTask_ReleaseEngagementSlotInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr TargetActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActor");
        TargetActor_Offset = CallGetPropertyOffset(TargetActor_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeTask_ReleaseEngagementSlotInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            TargetActor = ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(InNativeStruct + TargetActor_Offset, 0);
        }
    }
    
    public static FTsnStateTreeTask_ReleaseEngagementSlotInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeTask_ReleaseEngagementSlotInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(buffer + TargetActor_Offset, 0, TargetActor);
        }
    }
}

public static class FTsnStateTreeTask_ReleaseEngagementSlotInstanceDataMarshaller
{
    public static FTsnStateTreeTask_ReleaseEngagementSlotInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeTask_ReleaseEngagementSlotInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeTask_ReleaseEngagementSlotInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeTask_ReleaseEngagementSlotInstanceData.NativeDataSize;
    }
}