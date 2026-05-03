using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 交战槽位组件 —— 挂载在可被攻击的目标身上。
/// 
/// 特性：
/// 1. 多环支持：不同攻击距离的攻击者站在不同的环上
/// 2. 角度分配：从请求者当前位置计算理想角度，在该角度附近
///    寻找不与同环已有攻击者冲突的空位
/// 3. 容错设计：槽位满时返回合理位置，不阻塞攻击行为
/// </summary>
[UClass, GeneratedType("TsnEngagementSlotComponent", "UnrealSharp.TireflySquadNavigation.TsnEngagementSlotComponent")]
public partial class UTsnEngagementSlotComponent : UnrealSharp.Engine.UActorComponent
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnEngagementSlotComponent).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnEngagementSlotComponent");
    static UTsnEngagementSlotComponent()
    {
        IntPtr MaxSlots_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "MaxSlots");
        MaxSlots_Offset = CallGetPropertyOffset(MaxSlots_NativeProperty);
        IntPtr SlotRadiusOffset_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SlotRadiusOffset");
        SlotRadiusOffset_Offset = CallGetPropertyOffset(SlotRadiusOffset_NativeProperty);
        IntPtr MinSlotSpacing_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "MinSlotSpacing");
        MinSlotSpacing_Offset = CallGetPropertyOffset(MinSlotSpacing_NativeProperty);
        IntPtr SameRingRadiusTolerance_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SameRingRadiusTolerance");
        SameRingRadiusTolerance_Offset = CallGetPropertyOffset(SameRingRadiusTolerance_NativeProperty);
        IntPtr bDrawDebugSlots_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDrawDebugSlots");
        bDrawDebugSlots_Offset = CallGetPropertyOffset(bDrawDebugSlots_NativeProperty);
        Slots_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "Slots");
        Slots_Offset = CallGetPropertyOffset(Slots_NativeProperty);
        GetOccupiedSlotCount_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetOccupiedSlotCount");
        GetOccupiedSlotCount_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetOccupiedSlotCount_NativeFunction);
        GetOccupiedSlotCount_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetOccupiedSlotCount_NativeFunction, "ReturnValue");
        RequestSlot_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "RequestSlot");
        RequestSlot_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(RequestSlot_NativeFunction);
        RequestSlot_Requester_Offset = CallGetPropertyOffsetFromName(RequestSlot_NativeFunction, "Requester");
        RequestSlot_AttackRange_Offset = CallGetPropertyOffsetFromName(RequestSlot_NativeFunction, "AttackRange");
        RequestSlot_ReturnValue_Offset = CallGetPropertyOffsetFromName(RequestSlot_NativeFunction, "ReturnValue");
        ReleaseSlot_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "ReleaseSlot");
        ReleaseSlot_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(ReleaseSlot_NativeFunction);
        ReleaseSlot_Requester_Offset = CallGetPropertyOffsetFromName(ReleaseSlot_NativeFunction, "Requester");
        HasSlot_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "HasSlot");
        HasSlot_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(HasSlot_NativeFunction);
        HasSlot_Requester_Offset = CallGetPropertyOffsetFromName(HasSlot_NativeFunction, "Requester");
        HasSlot_ReturnValue_Offset = CallGetPropertyOffsetFromName(HasSlot_NativeFunction, "ReturnValue");
        IsSlotAvailable_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "IsSlotAvailable");
        IsSlotAvailable_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(IsSlotAvailable_NativeFunction);
        IsSlotAvailable_Requester_Offset = CallGetPropertyOffsetFromName(IsSlotAvailable_NativeFunction, "Requester");
        IsSlotAvailable_ReturnValue_Offset = CallGetPropertyOffsetFromName(IsSlotAvailable_NativeFunction, "ReturnValue");
        OnOwnerReleased_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "OnOwnerReleased");
    }
    
    static int MaxSlots_Offset;
    
    /// <summary>
    /// 最大同时攻击者数
    /// </summary>
    public int MaxSlots
    {
        get
        {
            return BlittableMarshaller<int>.FromNative(NativeObject + MaxSlots_Offset, 0);
        }
        set
        {
            BlittableMarshaller<int>.ToNative(NativeObject + MaxSlots_Offset, 0, value);
        }
    }
    
    
    static int SlotRadiusOffset_Offset;
    
    /// <summary>
    /// 槽位半径相对于攻击距离的偏移（负值表示站位比攻击距离稍近）。
    /// 确保站在槽位上时严格在攻击范围内。
    /// </summary>
    public float SlotRadiusOffset
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + SlotRadiusOffset_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + SlotRadiusOffset_Offset, 0, value);
        }
    }
    
    
    static int MinSlotSpacing_Offset;
    
    /// <summary>
    /// 同环上两个攻击者之间的最小间距（世界单位）。
    /// 用于计算最小角度间隔。
    /// </summary>
    public float MinSlotSpacing
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + MinSlotSpacing_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + MinSlotSpacing_Offset, 0, value);
        }
    }
    
    
    static int SameRingRadiusTolerance_Offset;
    
    /// <summary>
    /// 半径差在此范围内的攻击者被视为"同一环"，需做角度冲突检测。
    /// 建议设为碰撞胶囊直径左右。
    /// </summary>
    public float SameRingRadiusTolerance
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + SameRingRadiusTolerance_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + SameRingRadiusTolerance_Offset, 0, value);
        }
    }
    
    
    static int bDrawDebugSlots_Offset;
    
    /// <summary>
    /// 是否绘制槽位调试信息。
    /// 在 Shipping 配置中此属性仍存在但实际绘制代码不会编译进去。
    /// </summary>
    public bool DrawDebugSlots
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDrawDebugSlots_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDrawDebugSlots_Offset, 0, value);
        }
    }
    
    
    static int Slots_Offset;
    static IntPtr Slots_NativeProperty;
    ArrayMarshaller<UnrealSharp.TireflySquadNavigation.FTsnEngagementSlotInfo> Slots_Marshaller = null;
    
    private System.Collections.Generic.IList<UnrealSharp.TireflySquadNavigation.FTsnEngagementSlotInfo> Slots
    {
        get
        {
            Slots_Marshaller ??= new ArrayMarshaller<UnrealSharp.TireflySquadNavigation.FTsnEngagementSlotInfo>(Slots_NativeProperty, StructMarshaller<UnrealSharp.TireflySquadNavigation.FTsnEngagementSlotInfo>.ToNative, StructMarshaller<UnrealSharp.TireflySquadNavigation.FTsnEngagementSlotInfo>.FromNative);
            return Slots_Marshaller.FromNative(NativeObject + Slots_Offset, 0);
        }
    }
    
    
    // GetOccupiedSlotCount
    static IntPtr GetOccupiedSlotCount_NativeFunction;
    static int GetOccupiedSlotCount_ParamsSize;
    static int GetOccupiedSlotCount_ReturnValue_Offset;
    
    public int OccupiedSlotCount
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetOccupiedSlotCount_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetOccupiedSlotCount_NativeFunction, paramsBuffer, paramsBuffer + GetOccupiedSlotCount_ReturnValue_Offset);
                
                var returnValue = BlittableMarshaller<int>.FromNative(paramsBuffer + GetOccupiedSlotCount_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
    }
    
    // RequestSlot
    static IntPtr RequestSlot_NativeFunction;
    static int RequestSlot_ParamsSize;
    static int RequestSlot_Requester_Offset;
    static int RequestSlot_AttackRange_Offset;
    static int RequestSlot_ReturnValue_Offset;
    
    /// <summary>
    /// 请求一个交战槽位。
    /// 已有槽位则复用本地极坐标并返回当前帧的世界快照；
    /// 否则在对应环上分配角度最近的空位。
    /// 
    /// 
    /// </summary>
    [UFunction, GeneratedType("RequestSlot", "UnrealSharp.TireflySquadNavigation.RequestSlot")]
    public UnrealSharp.CoreUObject.FVector RequestSlot(UnrealSharp.Engine.AActor requester, float attackRange)
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[RequestSlot_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(paramsBuffer + RequestSlot_Requester_Offset, 0, requester);
            BlittableMarshaller<float>.ToNative(paramsBuffer + RequestSlot_AttackRange_Offset, 0, attackRange);
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, RequestSlot_NativeFunction, paramsBuffer, paramsBuffer + RequestSlot_ReturnValue_Offset);
            
            var returnValue = BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.FromNative(paramsBuffer + RequestSlot_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    
    // ReleaseSlot
    static IntPtr ReleaseSlot_NativeFunction;
    static int ReleaseSlot_ParamsSize;
    static int ReleaseSlot_Requester_Offset;
    
    /// <summary>
    /// 释放指定请求者持有的槽位。
    /// 攻击者死亡、切换目标或脱战时必须调用。
    /// 
    /// 
    /// </summary>
    [UFunction, GeneratedType("ReleaseSlot", "UnrealSharp.TireflySquadNavigation.ReleaseSlot")]
    public void ReleaseSlot(UnrealSharp.Engine.AActor requester)
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[ReleaseSlot_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(paramsBuffer + ReleaseSlot_Requester_Offset, 0, requester);
            
            UObjectExporter.CallInvokeNativeFunction(NativeObject, ReleaseSlot_NativeFunction, paramsBuffer, IntPtr.Zero);
            
        }
    }
    
    // HasSlot
    static IntPtr HasSlot_NativeFunction;
    static int HasSlot_ParamsSize;
    static int HasSlot_Requester_Offset;
    static int HasSlot_ReturnValue_Offset;
    
    /// <summary>
    /// 查询指定 Actor 是否持有槽位。
    /// 
    /// 
    /// </summary>
    [UFunction, GeneratedType("HasSlot", "UnrealSharp.TireflySquadNavigation.HasSlot")]
    public bool HasSlot(UnrealSharp.Engine.AActor requester)
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[HasSlot_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(paramsBuffer + HasSlot_Requester_Offset, 0, requester);
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, HasSlot_NativeFunction, paramsBuffer, paramsBuffer + HasSlot_ReturnValue_Offset);
            
            var returnValue = BoolMarshaller.FromNative(paramsBuffer + HasSlot_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    
    // IsSlotAvailable
    static IntPtr IsSlotAvailable_NativeFunction;
    static int IsSlotAvailable_ParamsSize;
    static int IsSlotAvailable_Requester_Offset;
    static int IsSlotAvailable_ReturnValue_Offset;
    
    /// <summary>
    /// 查询当前是否有可分配的槽位。
    /// BT Task 在调用 RequestSlot 前应先通过此方法检查；
    /// 槽满时直接返回 Failed，让行为树切换攻击目标。
    /// 
    /// 
    /// </summary>
    [UFunction, GeneratedType("IsSlotAvailable", "UnrealSharp.TireflySquadNavigation.IsSlotAvailable")]
    public bool IsSlotAvailable(UnrealSharp.Engine.AActor requester)
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[IsSlotAvailable_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(paramsBuffer + IsSlotAvailable_Requester_Offset, 0, requester);
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, IsSlotAvailable_NativeFunction, paramsBuffer, paramsBuffer + IsSlotAvailable_ReturnValue_Offset);
            
            var returnValue = BoolMarshaller.FromNative(paramsBuffer + IsSlotAvailable_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    
    // OnOwnerReleased
    static IntPtr OnOwnerReleased_NativeFunction;
    
    /// <summary>
    /// 对象池释放回调。宿主项目在将此目标返回对象池时调用。
    /// 释放所有槽位并通知 TsnEngagementSlotSubsystem 清除关联的占用记录。
    /// </summary>
    [UFunction, GeneratedType("OnOwnerReleased", "UnrealSharp.TireflySquadNavigation.OnOwnerReleased")]
    public void OnOwnerReleased()
    {
        unsafe
        {
            UObjectExporter.CallInvokeNativeFunction(NativeObject, OnOwnerReleased_NativeFunction, IntPtr.Zero, IntPtr.Zero);
        }
    }
    
    
}