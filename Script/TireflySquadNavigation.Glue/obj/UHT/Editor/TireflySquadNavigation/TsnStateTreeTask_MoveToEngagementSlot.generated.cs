using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 在攻击距离外申请目标的交战槽位并接近其世界快照。
/// 
/// 退出条件（与 BT 版本对齐）：
/// - 到达槽位 → Succeeded（保留槽位）
/// - 中途进入攻击距离 → Succeeded（保留槽位）
/// - MoveTo 失败 / 目标失效 / 超时 → Failed（释放槽位）
/// - 状态被外部中断 → ExitState 释放槽位
/// </summary>
[UStruct, GeneratedType("TsnStateTreeTask_MoveToEngagementSlot", "UnrealSharp.TireflySquadNavigation.TsnStateTreeTask_MoveToEngagementSlot")]
public partial struct FTsnStateTreeTask_MoveToEngagementSlot : MarshalledStruct<FTsnStateTreeTask_MoveToEngagementSlot>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeTask_MoveToEngagementSlot).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeTask_MoveToEngagementSlot");
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
    
    // TaskEnabled
    
    static byte bTaskEnabled_FieldMask;
    static int bTaskEnabled_Offset;
    /// <summary>
    /// True if the node is Enabled (i.e. not explicitly disabled in the asset).
    /// </summary>
    public bool TaskEnabled;
    
    // TransitionHandlingPriority
    
    static int TransitionHandlingPriority_Offset;
    public UnrealSharp.StateTreeModule.EStateTreeTransitionPriority TransitionHandlingPriority;
    
    // ConsideredForCompletion
    
    static byte bConsideredForCompletion_FieldMask;
    static int bConsideredForCompletion_Offset;
    /// <summary>
    /// True if the task is considered for completion.
    /// False if the task runs in the background without affecting the state completion.
    /// </summary>
    public bool ConsideredForCompletion;
    
    // CanEditConsideredForCompletion
    
    static byte bCanEditConsideredForCompletion_FieldMask;
    static int bCanEditConsideredForCompletion_Offset;
    /// <summary>
    /// True if the user can edit bConsideredForCompletion in the editor.
    /// </summary>
    public bool CanEditConsideredForCompletion;
    
    public FTsnStateTreeTask_MoveToEngagementSlot(FName Name, UnrealSharp.StateTreeModule.FStateTreeIndex16 BindingsBatch, UnrealSharp.StateTreeModule.FStateTreeIndex16 OutputBindingsBatch, UnrealSharp.StateTreeModule.FStateTreeIndex16 InstanceTemplateIndex, UnrealSharp.StateTreeModule.FStateTreeIndex16 ExecutionRuntimeTemplateIndex, UnrealSharp.StateTreeModule.FStateTreeDataHandle InstanceDataHandle, bool TaskEnabled, UnrealSharp.StateTreeModule.EStateTreeTransitionPriority TransitionHandlingPriority, bool ConsideredForCompletion, bool CanEditConsideredForCompletion)
    {
        this.Name = Name;
        this.BindingsBatch = BindingsBatch;
        this.OutputBindingsBatch = OutputBindingsBatch;
        this.InstanceTemplateIndex = InstanceTemplateIndex;
        this.ExecutionRuntimeTemplateIndex = ExecutionRuntimeTemplateIndex;
        this.InstanceDataHandle = InstanceDataHandle;
        this.TaskEnabled = TaskEnabled;
        this.TransitionHandlingPriority = TransitionHandlingPriority;
        this.ConsideredForCompletion = ConsideredForCompletion;
        this.CanEditConsideredForCompletion = CanEditConsideredForCompletion;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeTask_MoveToEngagementSlot()
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
        bTaskEnabled_FieldMask = CallGetBoolPropertyFieldMaskFromName(NativeClassPtr, "bTaskEnabled");
        IntPtr bTaskEnabled_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bTaskEnabled");
        bTaskEnabled_Offset = CallGetPropertyOffset(bTaskEnabled_NativeProperty);
        IntPtr TransitionHandlingPriority_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TransitionHandlingPriority");
        TransitionHandlingPriority_Offset = CallGetPropertyOffset(TransitionHandlingPriority_NativeProperty);
        #if WITH_EDITOR
        bConsideredForCompletion_FieldMask = CallGetBoolPropertyFieldMaskFromName(NativeClassPtr, "bConsideredForCompletion");
        IntPtr bConsideredForCompletion_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bConsideredForCompletion");
        bConsideredForCompletion_Offset = CallGetPropertyOffset(bConsideredForCompletion_NativeProperty);
        #endif
        #if WITH_EDITOR
        bCanEditConsideredForCompletion_FieldMask = CallGetBoolPropertyFieldMaskFromName(NativeClassPtr, "bCanEditConsideredForCompletion");
        IntPtr bCanEditConsideredForCompletion_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bCanEditConsideredForCompletion");
        bCanEditConsideredForCompletion_Offset = CallGetPropertyOffset(bCanEditConsideredForCompletion_NativeProperty);
        #endif
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeTask_MoveToEngagementSlot(IntPtr InNativeStruct)
    {
        unsafe
        {
            Name = BlittableMarshaller<FName>.FromNative(InNativeStruct + Name_Offset, 0);
            BindingsBatch = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + BindingsBatch_Offset, 0);
            OutputBindingsBatch = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + OutputBindingsBatch_Offset, 0);
            InstanceTemplateIndex = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + InstanceTemplateIndex_Offset, 0);
            ExecutionRuntimeTemplateIndex = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeIndex16>.FromNative(InNativeStruct + ExecutionRuntimeTemplateIndex_Offset, 0);
            InstanceDataHandle = StructMarshaller<UnrealSharp.StateTreeModule.FStateTreeDataHandle>.FromNative(InNativeStruct + InstanceDataHandle_Offset, 0);
            TaskEnabled = BitfieldBoolMarshaller.FromNative(InNativeStruct + bTaskEnabled_Offset, bTaskEnabled_FieldMask);
            TransitionHandlingPriority = EnumMarshaller<UnrealSharp.StateTreeModule.EStateTreeTransitionPriority>.FromNative(InNativeStruct + TransitionHandlingPriority_Offset, 0);
            #if WITH_EDITOR
            ConsideredForCompletion = BitfieldBoolMarshaller.FromNative(InNativeStruct + bConsideredForCompletion_Offset, bConsideredForCompletion_FieldMask);
            #endif
            #if WITH_EDITOR
            CanEditConsideredForCompletion = BitfieldBoolMarshaller.FromNative(InNativeStruct + bCanEditConsideredForCompletion_Offset, bCanEditConsideredForCompletion_FieldMask);
            #endif
        }
    }
    
    public static FTsnStateTreeTask_MoveToEngagementSlot FromNative(IntPtr buffer) => new FTsnStateTreeTask_MoveToEngagementSlot(buffer);
    
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
            BitfieldBoolMarshaller.ToNative(buffer + bTaskEnabled_Offset, bTaskEnabled_FieldMask, TaskEnabled);
            EnumMarshaller<UnrealSharp.StateTreeModule.EStateTreeTransitionPriority>.ToNative(buffer + TransitionHandlingPriority_Offset, 0, TransitionHandlingPriority);
            #if WITH_EDITOR
            BitfieldBoolMarshaller.ToNative(buffer + bConsideredForCompletion_Offset, bConsideredForCompletion_FieldMask, ConsideredForCompletion);
            #endif
            #if WITH_EDITOR
            BitfieldBoolMarshaller.ToNative(buffer + bCanEditConsideredForCompletion_Offset, bCanEditConsideredForCompletion_FieldMask, CanEditConsideredForCompletion);
            #endif
        }
    }
}

public static class FTsnStateTreeTask_MoveToEngagementSlotMarshaller
{
    public static FTsnStateTreeTask_MoveToEngagementSlot FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeTask_MoveToEngagementSlot(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeTask_MoveToEngagementSlot obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeTask_MoveToEngagementSlot.NativeDataSize;
    }
}