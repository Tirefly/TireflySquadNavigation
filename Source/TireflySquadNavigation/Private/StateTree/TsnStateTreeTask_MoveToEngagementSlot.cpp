// TsnStateTreeTask_MoveToEngagementSlot.cpp

#include "StateTree/TsnStateTreeTask_MoveToEngagementSlot.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "TsnLog.h"

namespace
{
	/**
	 * 申请槽位并发�?MoveTo；成功时设置 bSlotAcquired，失败返�?false（已主动释放任何已申请的槽位）�?
	 */
	static bool RequestSlotAndMove(
		FTsnStateTreeTask_MoveToEngagementSlotInstanceData& Data)
	{
		AAIController* AICon = Data.AIController;
		AActor* Target = Data.TargetActor;
		if (!AICon || !AICon->GetPawn() || !Target)
		{
			return false;
		}

		UTsnEngagementSlotComponent* SlotComp =
			Target->FindComponentByClass<UTsnEngagementSlotComponent>();
		if (!SlotComp)
		{
			return false;
		}

		if (!SlotComp->IsSlotAvailable(AICon->GetPawn()))
		{
			UE_LOG(LogTireflySquadNav, Log,
				TEXT("TsnStateTreeTask_MoveToEngagementSlot: target [%s] slots full."),
				*Target->GetName());
			return false;
		}

		Data.CachedSlotPosition = SlotComp->RequestSlot(
			AICon->GetPawn(), Data.CachedEngagementRange);
		Data.HeldSlotComponent = SlotComp;
		Data.bSlotAcquired = true;

		FAIMoveRequest MoveReq;
		MoveReq.SetGoalLocation(Data.CachedSlotPosition);
		MoveReq.SetAcceptanceRadius(Data.AcceptanceRadius);
		MoveReq.SetUsePathfinding(true);
		MoveReq.SetReachTestIncludesAgentRadius(false);

		const FPathFollowingRequestResult Result = AICon->MoveTo(MoveReq);
		Data.CachedTargetLocation = Target->GetActorLocation();
		Data.TimeSinceLastRePathCheck = 0.f;

		if (Result.Code == EPathFollowingRequestResult::Failed)
		{
			SlotComp->ReleaseSlot(AICon->GetPawn());
			Data.HeldSlotComponent.Reset();
			Data.bSlotAcquired = false;
			return false;
		}
		return true;
	}

	static void ReleaseHeldSlot(
		FTsnStateTreeTask_MoveToEngagementSlotInstanceData& Data)
	{
		if (!Data.bSlotAcquired)
		{
			return;
		}
		UTsnEngagementSlotComponent* SlotComp = Data.HeldSlotComponent.Get();
		AAIController* AICon = Data.AIController;
		if (SlotComp && AICon && AICon->GetPawn())
		{
			SlotComp->ReleaseSlot(AICon->GetPawn());
		}
		Data.HeldSlotComponent.Reset();
		Data.bSlotAcquired = false;
	}
}

FTsnStateTreeTask_MoveToEngagementSlot::FTsnStateTreeTask_MoveToEngagementSlot()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FTsnStateTreeTask_MoveToEngagementSlot::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	AAIController* AICon = Data.AIController;
	if (!AICon || !AICon->GetPawn() || !Data.TargetActor)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_MoveToEngagementSlot: missing AIController/Pawn/Target."));
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn->Implements<UTsnTacticalUnit>())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_MoveToEngagementSlot: Pawn [%s] does not implement ITsnTacticalUnit."),
			*Pawn->GetName());
		return EStateTreeRunStatus::Failed;
	}

	const float EngagementRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
	if (EngagementRange <= 0.f)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_MoveToEngagementSlot: invalid EngagementRange %.1f."),
			EngagementRange);
		return EStateTreeRunStatus::Failed;
	}
	Data.CachedEngagementRange = EngagementRange;

	// 已在攻击距离�?�?直接成功，无需申请槽位
	if (FVector::Dist2D(Pawn->GetActorLocation(), Data.TargetActor->GetActorLocation())
		<= EngagementRange)
	{
		Data.bExitedWithSuccess = true;
		return EStateTreeRunStatus::Succeeded;
	}

	Data.ElapsedApproachTime = 0.f;
	Data.bExitedWithSuccess = false;
	Data.bSlotAcquired = false;
	Data.HeldSlotComponent.Reset();

	if (!RequestSlotAndMove(Data))
	{
		return EStateTreeRunStatus::Failed;
	}
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FTsnStateTreeTask_MoveToEngagementSlot::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	AAIController* AICon = Data.AIController;
	if (!AICon || !AICon->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* Target = Data.TargetActor;
	if (!Target)
	{
		AICon->StopMovement();
		return EStateTreeRunStatus::Failed;
	}

	const FVector PawnLoc = AICon->GetPawn()->GetActorLocation();

	// 退出条�?(a)：到达槽�?
	if (FVector::Dist2D(PawnLoc, Data.CachedSlotPosition) <= Data.AcceptanceRadius)
	{
		AICon->StopMovement();
		Data.bExitedWithSuccess = true;
		return EStateTreeRunStatus::Succeeded;
	}

	// 退出条�?(b)：中途进入攻击距�?
	if (FVector::Dist2D(PawnLoc, Target->GetActorLocation()) <= Data.CachedEngagementRange)
	{
		AICon->StopMovement();
		Data.bExitedWithSuccess = true;
		return EStateTreeRunStatus::Succeeded;
	}

	// 退出条�?(c)：超时安全兜�?
	Data.ElapsedApproachTime += DeltaTime;
	if (Data.MaxApproachTime > 0.f && Data.ElapsedApproachTime >= Data.MaxApproachTime)
	{
		AICon->StopMovement();
		return EStateTreeRunStatus::Failed;
	}

	// 周期性刷新槽位快照与重新寻路
	Data.TimeSinceLastRePathCheck += DeltaTime;
	if (Data.TimeSinceLastRePathCheck >= Data.RePathCheckInterval)
	{
		Data.TimeSinceLastRePathCheck = 0.f;
		const bool bTargetMoved =
			FVector::Dist2D(Data.CachedTargetLocation, Target->GetActorLocation())
			> Data.RePathDistanceThreshold;
		const bool bPathStopped = AICon->GetMoveStatus() != EPathFollowingStatus::Moving;
		if (bTargetMoved || bPathStopped)
		{
			if (!RequestSlotAndMove(Data))
			{
				AICon->StopMovement();
				return EStateTreeRunStatus::Failed;
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FTsnStateTreeTask_MoveToEngagementSlot::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	// 槽位生命周期：成功保留，失败/中断释放
	const bool bSucceeded = Data.bExitedWithSuccess;
	if (!bSucceeded)
	{
		ReleaseHeldSlot(Data);
		if (Data.AIController)
		{
			Data.AIController->StopMovement();
		}
	}

	// 重置成功标志，便于下一次进入状�?
	Data.bExitedWithSuccess = false;
}
