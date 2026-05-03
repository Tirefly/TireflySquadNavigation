using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// Attack 任务的实例数据
/// </summary>
[UStruct, GeneratedType("TsnTestStateTreeTask_AttackInstanceData", "UnrealSharp.TireflySquadNavigationTest.TsnTestStateTreeTask_AttackInstanceData")]
public partial struct FTsnTestStateTreeTask_AttackInstanceData : MarshalledStruct<FTsnTestStateTreeTask_AttackInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnTestStateTreeTask_AttackInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestStateTreeTask_AttackInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定的 AIController（由 AI Schema 自动注入）
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    // AttackDuration
    
    static int AttackDuration_Offset;
    /// <summary>
    /// 攻击持续时长（秒）
    /// </summary>
    public float AttackDuration;
    
    // ElapsedTime
    
    static int ElapsedTime_Offset;
    /// <summary>
    /// 运行时累计经过时间（秒）
    /// </summary>
    public float ElapsedTime;
    
    public FTsnTestStateTreeTask_AttackInstanceData(UnrealSharp.AIModule.AAIController AIController, float AttackDuration, float ElapsedTime)
    {
        this.AIController = AIController;
        this.AttackDuration = AttackDuration;
        this.ElapsedTime = ElapsedTime;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnTestStateTreeTask_AttackInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr AttackDuration_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AttackDuration");
        AttackDuration_Offset = CallGetPropertyOffset(AttackDuration_NativeProperty);
        IntPtr ElapsedTime_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ElapsedTime");
        ElapsedTime_Offset = CallGetPropertyOffset(ElapsedTime_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnTestStateTreeTask_AttackInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            AttackDuration = BlittableMarshaller<float>.FromNative(InNativeStruct + AttackDuration_Offset, 0);
            ElapsedTime = BlittableMarshaller<float>.FromNative(InNativeStruct + ElapsedTime_Offset, 0);
        }
    }
    
    public static FTsnTestStateTreeTask_AttackInstanceData FromNative(IntPtr buffer) => new FTsnTestStateTreeTask_AttackInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            BlittableMarshaller<float>.ToNative(buffer + AttackDuration_Offset, 0, AttackDuration);
            BlittableMarshaller<float>.ToNative(buffer + ElapsedTime_Offset, 0, ElapsedTime);
        }
    }
}

public static class FTsnTestStateTreeTask_AttackInstanceDataMarshaller
{
    public static FTsnTestStateTreeTask_AttackInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnTestStateTreeTask_AttackInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnTestStateTreeTask_AttackInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnTestStateTreeTask_AttackInstanceData.NativeDataSize;
    }
}