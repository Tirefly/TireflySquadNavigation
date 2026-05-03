using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UStruct, GeneratedType("TsnStateTreeCondition_IsTargetStationary", "UnrealSharp.TireflySquadNavigation.TsnStateTreeCondition_IsTargetStationary")]
public partial struct FTsnStateTreeCondition_IsTargetStationary : MarshalledStruct<FTsnStateTreeCondition_IsTargetStationary>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeCondition_IsTargetStationary).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeCondition_IsTargetStationary");
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
    
    // Operand
    
    static int Operand_Offset;
    public UnrealSharp.StateTreeModule.EStateTreeExpressionOperand Operand;
    
    // DeltaIndent
    
    static int DeltaIndent_Offset;
    public sbyte DeltaIndent;
    
    // EvaluationMode
    
    static int EvaluationMode_Offset;
    public UnrealSharp.StateTreeModule.EStateTreeConditionEvaluationMode EvaluationMode;
    
    public FTsnStateTreeCondition_IsTargetStationary(FName Name, UnrealSharp.StateTreeModule.FStateTreeIndex16 BindingsBatch, UnrealSharp.StateTreeModule.FStateTreeIndex16 OutputBindingsBatch, UnrealSharp.StateTreeModule.FStateTreeIndex16 InstanceTemplateIndex, UnrealSharp.StateTreeModule.FStateTreeIndex16 ExecutionRuntimeTemplateIndex, UnrealSharp.StateTreeModule.FStateTreeDataHandle InstanceDataHandle, UnrealSharp.StateTreeModule.EStateTreeExpressionOperand Operand, sbyte DeltaIndent, UnrealSharp.StateTreeModule.EStateTreeConditionEvaluationMode EvaluationMode)
    {
        this.Name = Name;
        this.BindingsBatch = BindingsBatch;
        this.OutputBindingsBatch = OutputBindingsBatch;
        this.InstanceTemplateIndex = InstanceTemplateIndex;
        this.ExecutionRuntimeTemplateIndex = ExecutionRuntimeTemplateIndex;
        this.InstanceDataHandle = InstanceDataHandle;
        this.Operand = Operand;
        this.DeltaIndent = DeltaIndent;
        this.EvaluationMode = EvaluationMode;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeCondition_IsTargetStationary()
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
        IntPtr Operand_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "Operand");
        Operand_Offset = CallGetPropertyOffset(Operand_NativeProperty);
        IntPtr DeltaIndent_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DeltaIndent");
        DeltaIndent_Offset = CallGetPropertyOffset(DeltaIndent_NativeProperty);
        IntPtr EvaluationMode_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EvaluationMode");
        EvaluationMode_Offset = CallGetPropertyOffset(EvaluationMode_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeCondition_IsTargetStationary(IntPtr InNativeStruct)
    {
        unsafe
        {
            Name = BlittableMarshaller<FName>.FromNative(InNativeStruct + Name_Offset, 0);
            BindingsBatch = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + BindingsBatch_Offset, 0);
            OutputBindingsBatch = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + OutputBindingsBatch_Offset, 0);
            InstanceTemplateIndex = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + InstanceTemplateIndex_Offset, 0);
            ExecutionRuntimeTemplateIndex = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + ExecutionRuntimeTemplateIndex_Offset, 0);
            InstanceDataHandle = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeDataHandle>.FromNative(InNativeStruct + InstanceDataHandle_Offset, 0);
            Operand = EnumMarshaller<UnrealSharp.StateTreeModule.EStateTreeExpressionOperand>.FromNative(InNativeStruct + Operand_Offset, 0);
            DeltaIndent = BlittableMarshaller<sbyte>.FromNative(InNativeStruct + DeltaIndent_Offset, 0);
            EvaluationMode = EnumMarshaller<UnrealSharp.StateTreeModule.EStateTreeConditionEvaluationMode>.FromNative(InNativeStruct + EvaluationMode_Offset, 0);
        }
    }
    
    public static FTsnStateTreeCondition_IsTargetStationary FromNative(IntPtr buffer) => new FTsnStateTreeCondition_IsTargetStationary(buffer);
    
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
            EnumMarshaller<UnrealSharp.StateTreeModule.EStateTreeExpressionOperand>.ToNative(buffer + Operand_Offset, 0, Operand);
            BlittableMarshaller<sbyte>.ToNative(buffer + DeltaIndent_Offset, 0, DeltaIndent);
            EnumMarshaller<UnrealSharp.StateTreeModule.EStateTreeConditionEvaluationMode>.ToNative(buffer + EvaluationMode_Offset, 0, EvaluationMode);
        }
    }
}

public static class FTsnStateTreeCondition_IsTargetStationaryMarshaller
{
    public static FTsnStateTreeCondition_IsTargetStationary FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeCondition_IsTargetStationary(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeCondition_IsTargetStationary obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeCondition_IsTargetStationary.NativeDataSize;
    }
}