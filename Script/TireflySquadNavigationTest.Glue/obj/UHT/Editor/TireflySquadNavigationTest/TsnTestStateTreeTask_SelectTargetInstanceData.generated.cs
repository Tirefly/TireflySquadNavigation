using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// SelectTarget 任务的实例数据
/// </summary>
[UStruct, GeneratedType("TsnTestStateTreeTask_SelectTargetInstanceData", "UnrealSharp.TireflySquadNavigationTest.TsnTestStateTreeTask_SelectTargetInstanceData")]
public partial struct FTsnTestStateTreeTask_SelectTargetInstanceData : MarshalledStruct<FTsnTestStateTreeTask_SelectTargetInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnTestStateTreeTask_SelectTargetInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestStateTreeTask_SelectTargetInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定的 AIController（由 AI Schema 自动注入）
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    // SearchRadius
    
    static int SearchRadius_Offset;
    /// <summary>
    /// 最大搜索半径（cm）
    /// </summary>
    public float SearchRadius;
    
    // TargetActor
    
    static int TargetActor_Offset;
    /// <summary>
    /// 输出：选中的目标（可为 null）
    /// </summary>
    public UnrealSharp.Engine.AActor TargetActor;
    
    // HasTarget
    
    static int bHasTarget_Offset;
    /// <summary>
    /// 输出：是否找到目标
    /// </summary>
    public bool HasTarget;
    
    public FTsnTestStateTreeTask_SelectTargetInstanceData(UnrealSharp.AIModule.AAIController AIController, float SearchRadius, UnrealSharp.Engine.AActor TargetActor, bool HasTarget)
    {
        this.AIController = AIController;
        this.SearchRadius = SearchRadius;
        this.TargetActor = TargetActor;
        this.HasTarget = HasTarget;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnTestStateTreeTask_SelectTargetInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr SearchRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SearchRadius");
        SearchRadius_Offset = CallGetPropertyOffset(SearchRadius_NativeProperty);
        IntPtr TargetActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetActor");
        TargetActor_Offset = CallGetPropertyOffset(TargetActor_NativeProperty);
        IntPtr bHasTarget_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bHasTarget");
        bHasTarget_Offset = CallGetPropertyOffset(bHasTarget_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnTestStateTreeTask_SelectTargetInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            SearchRadius = BlittableMarshaller<float>.FromNative(InNativeStruct + SearchRadius_Offset, 0);
            TargetActor = ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(InNativeStruct + TargetActor_Offset, 0);
            HasTarget = BoolMarshaller.FromNative(InNativeStruct + bHasTarget_Offset, 0);
        }
    }
    
    public static FTsnTestStateTreeTask_SelectTargetInstanceData FromNative(IntPtr buffer) => new FTsnTestStateTreeTask_SelectTargetInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            BlittableMarshaller<float>.ToNative(buffer + SearchRadius_Offset, 0, SearchRadius);
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(buffer + TargetActor_Offset, 0, TargetActor);
            BoolMarshaller.ToNative(buffer + bHasTarget_Offset, 0, HasTarget);
        }
    }
}

public static class FTsnTestStateTreeTask_SelectTargetInstanceDataMarshaller
{
    public static FTsnTestStateTreeTask_SelectTargetInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnTestStateTreeTask_SelectTargetInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnTestStateTreeTask_SelectTargetInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnTestStateTreeTask_SelectTargetInstanceData.NativeDataSize;
    }
}