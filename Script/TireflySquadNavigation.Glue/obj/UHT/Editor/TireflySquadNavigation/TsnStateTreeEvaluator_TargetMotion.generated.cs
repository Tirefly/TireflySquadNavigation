using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 目标运动评估器：每帧暴露 2D 速度与“是否静止”观测量。
/// </summary>
[UStruct, GeneratedType("TsnStateTreeEvaluator_TargetMotion", "UnrealSharp.TireflySquadNavigation.TsnStateTreeEvaluator_TargetMotion")]
public partial struct FTsnStateTreeEvaluator_TargetMotion : MarshalledStruct<FTsnStateTreeEvaluator_TargetMotion>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeEvaluator_TargetMotion).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeEvaluator_TargetMotion");
    // Name
    
    static int Name_Offset;
    /// <summary>
    /// Name of the node.
    /// </summary>
    public FName Name;
    
    // BindingsBatch
    
    static int BindingsBatch_Offset;
    /// <summary>
    /// Property binding copy batch handle.
    /// </summary>
    public UnrealSharp.StateTreeModule.FStateTreeIndex16 BindingsBatch;
    
    // OutputBindingsBatch
    
    static int OutputBindingsBatch_Offset;
    /// <summary>
    /// Property output binding copy batch handle.
    /// </summary>
    public UnrealSharp.StateTreeModule.FStateTreeIndex16 OutputBindingsBatch;
    
    // InstanceTemplateIndex
    
    static int InstanceTemplateIndex_Offset;
    /// <summary>
    /// Index of template instance data for the node. Can point to Shared or Default instance data in StateTree depending on node type.
    /// </summary>
    public UnrealSharp.StateTreeModule.FStateTreeIndex16 InstanceTemplateIndex;
    
    // ExecutionRuntimeTemplateIndex
    
    static int ExecutionRuntimeTemplateIndex_Offset;
    /// <summary>
    /// Index of template execution runtime data for the node.
    /// </summary>
    public UnrealSharp.StateTreeModule.FStateTreeIndex16 ExecutionRuntimeTemplateIndex;
    
    // InstanceDataHandle
    
    static int InstanceDataHandle_Offset;
    /// <summary>
    /// Data handle to access the instance data.
    /// </summary>
    public UnrealSharp.StateTreeModule.FStateTreeDataHandle InstanceDataHandle;
    
    public FTsnStateTreeEvaluator_TargetMotion(FName Name, UnrealSharp.StateTreeModule.FStateTreeIndex16 BindingsBatch, UnrealSharp.StateTreeModule.FStateTreeIndex16 OutputBindingsBatch, UnrealSharp.StateTreeModule.FStateTreeIndex16 InstanceTemplateIndex, UnrealSharp.StateTreeModule.FStateTreeIndex16 ExecutionRuntimeTemplateIndex, UnrealSharp.StateTreeModule.FStateTreeDataHandle InstanceDataHandle)
    {
        this.Name = Name;
        this.BindingsBatch = BindingsBatch;
        this.OutputBindingsBatch = OutputBindingsBatch;
        this.InstanceTemplateIndex = InstanceTemplateIndex;
        this.ExecutionRuntimeTemplateIndex = ExecutionRuntimeTemplateIndex;
        this.InstanceDataHandle = InstanceDataHandle;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeEvaluator_TargetMotion()
    {
        IntPtr Name_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "Name");
        Name_Offset = CallGetPropertyOffset(Name_NativeProperty);
        IntPtr BindingsBatch_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "BindingsBatch");
        BindingsBatch_Offset = CallGetPropertyOffset(BindingsBatch_NativeProperty);
        IntPtr OutputBindingsBatch_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "OutputBindingsBatch");
        OutputBindingsBatch_Offset = CallGetPropertyOffset(OutputBindingsBatch_NativeProperty);
        IntPtr InstanceTemplateIndex_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "InstanceTemplateIndex");
        InstanceTemplateIndex_Offset = CallGetPropertyOffset(InstanceTemplateIndex_NativeProperty);
        IntPtr ExecutionRuntimeTemplateIndex_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ExecutionRuntimeTemplateIndex");
        ExecutionRuntimeTemplateIndex_Offset = CallGetPropertyOffset(ExecutionRuntimeTemplateIndex_NativeProperty);
        IntPtr InstanceDataHandle_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "InstanceDataHandle");
        InstanceDataHandle_Offset = CallGetPropertyOffset(InstanceDataHandle_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeEvaluator_TargetMotion(IntPtr InNativeStruct)
    {
        unsafe
        {
            Name = BlittableMarshaller<FName>.FromNative(InNativeStruct + Name_Offset, 0);
            BindingsBatch = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + BindingsBatch_Offset, 0);
            OutputBindingsBatch = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + OutputBindingsBatch_Offset, 0);
            InstanceTemplateIndex = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + InstanceTemplateIndex_Offset, 0);
            ExecutionRuntimeTemplateIndex = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + ExecutionRuntimeTemplateIndex_Offset, 0);
            InstanceDataHandle = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeDataHandle>.FromNative(InNativeStruct + InstanceDataHandle_Offset, 0);
        }
    }
    
    public static FTsnStateTreeEvaluator_TargetMotion FromNative(IntPtr buffer) => new FTsnStateTreeEvaluator_TargetMotion(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            BlittableMarshaller<FName>.ToNative(buffer + Name_Offset, 0, Name);
            StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.ToNative(buffer + BindingsBatch_Offset, 0, BindingsBatch);
            StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.ToNative(buffer + OutputBindingsBatch_Offset, 0, OutputBindingsBatch);
            StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.ToNative(buffer + InstanceTemplateIndex_Offset, 0, InstanceTemplateIndex);
            StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.ToNative(buffer + ExecutionRuntimeTemplateIndex_Offset, 0, ExecutionRuntimeTemplateIndex);
            StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeDataHandle>.ToNative(buffer + InstanceDataHandle_Offset, 0, InstanceDataHandle);
        }
    }
}

public static class FTsnStateTreeEvaluator_TargetMotionMarshaller
{
    public static FTsnStateTreeEvaluator_TargetMotion FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeEvaluator_TargetMotion(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeEvaluator_TargetMotion obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeEvaluator_TargetMotion.NativeDataSize;
    }
}