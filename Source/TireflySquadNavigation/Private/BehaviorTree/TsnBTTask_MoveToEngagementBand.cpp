// TsnBTTask_MoveToEngagementBand.cpp

#include "BehaviorTree/TsnBTTask_MoveToEngagementBand.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "Navigation/PathFollowingComponent.h"
#include "TsnLog.h"

namespace TsnMoveToEngagementBandInternal
{
	// Crowd/PathFollowing 在接近接战带时可能会停在容差外几个单位内，
	// 给一个很小的抖动容差，避免任务在边缘距离上反复零意图恢复。
	constexpr float EngagementBandJitterTolerance = 5.f;

	// 接战带任务即使按“进入接战带”成功，也必须留在真实攻击窗口内，
	// 否则会出现任务结束后攻击装饰器立刻掉回移动分支的抖动。
	constexpr float EngagementBandAttackRangeGuard = 5.f;

	float ComputeEffectiveEngagementBandTolerance(
		float BaseAcceptanceRadius,
		float AttackRange,
		float BandRadius)
	{
		const float RequestedTolerance = FMath::Max(0.f, BaseAcceptanceRadius) + EngagementBandJitterTolerance;
		const float AttackRangeBudget = FMath::Max(0.f, AttackRange - BandRadius - EngagementBandAttackRangeGuard);
		return FMath::Min(RequestedTolerance, AttackRangeBudget);
	}

	float GetRequesterCollisionRadius(const APawn* Pawn)
	{
		return IsValid(Pawn) ? FMath::Max(0.f, Pawn->GetSimpleCollisionRadius()) : 0.f;
	}

	FVector ComputeEngagementNavigationGoal(
		const APawn* Pawn,
		const AActor* Target,
		const FVector& SlotPosition)
	{
		if (!IsValid(Pawn) || !IsValid(Target))
		{
			return SlotPosition;
		}

		const FVector TargetLocation = Target->GetActorLocation();
		FVector TargetToSlot = SlotPosition - TargetLocation;
		TargetToSlot.Z = 0.f;

		const float SlotRadius = TargetToSlot.Size();
		const float RequesterRadius = GetRequesterCollisionRadius(Pawn);
		if (SlotRadius <= KINDA_SMALL_NUMBER || RequesterRadius <= KINDA_SMALL_NUMBER)
		{
			return SlotPosition;
		}

		const float NavigationGoalRadius = FMath::Max(0.f, SlotRadius - RequesterRadius);
		return FVector(
			TargetLocation.X + TargetToSlot.X / SlotRadius * NavigationGoalRadius,
			TargetLocation.Y + TargetToSlot.Y / SlotRadius * NavigationGoalRadius,
			SlotPosition.Z);
	}

	void LogMoveToEngagementBandFailure(
		const AAIController* AICon,
		const AActor* Target,
		const TCHAR* Reason,
		float DistanceToTarget = -1.f,
		float BandRadius = -1.f,
		float EffectiveAcceptanceRadius = -1.f,
		float RequestedSpeed = -1.f,
		float ActualSpeed = -1.f)
	{
		const APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
		const UPathFollowingComponent* PathComp = AICon ? AICon->GetPathFollowingComponent() : nullptr;
		const FString PathStatusDesc = PathComp
			? PathComp->GetStatusDesc()
			: TEXT("NoPathFollowingComponent");
		const bool bHasValidPath = PathComp && PathComp->HasValidPath();

		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("MoveToEngagementBand FAILED: %s Pawn=%s Target=%s MoveStatus=%d PathStatus=%s HasValidPath=%s DistToTarget=%.1f BandRadius=%.1f Acceptance=%.1f RequestedSpeed=%.1f ActualSpeed=%.1f"),
			Reason,
			*GetNameSafe(Pawn),
			*GetNameSafe(Target),
			AICon ? static_cast<int32>(AICon->GetMoveStatus()) : -1,
			*PathStatusDesc,
			bHasValidPath ? TEXT("true") : TEXT("false"),
			DistanceToTarget,
			BandRadius,
			EffectiveAcceptanceRadius,
			RequestedSpeed,
			ActualSpeed);
	}
}

UTsnBTTask_MoveToEngagementBand::UTsnBTTask_MoveToEngagementBand()
{
	NodeName = TEXT("TSN Move To Engagement Band");
	bNotifyTick = true;

	// 每个 BehaviorTreeComponent 创建独立实例，类成员变量（含委托绑定）不会跨 Agent 互相覆盖
	bCreateNodeInstance = true;
}

void UTsnBTTask_MoveToEngagementBand::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		if (TargetKey.IsNone())
		{
			UE_LOG(LogTireflySquadNav, Warning,
				TEXT("TsnBTTask_MoveToEngagementBand: TargetKey '%s' not found in Blackboard '%s'; node will fail at runtime."),
				*TargetKey.SelectedKeyName.ToString(), *BBAsset->GetName());
		}
	}
	else
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_MoveToEngagementBand: TargetKey has no valid Blackboard asset; node will fail at runtime."));
	}
}

AActor* UTsnBTTask_MoveToEngagementBand::GetTargetActor(
	UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
}

bool UTsnBTTask_MoveToEngagementBand::RequestSlotAndMove(
	UBehaviorTreeComponent& OwnerComp, AActor* Target, bool bForceRefreshSlotAssignment)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("MoveToEngagementBand: RequestSlotAndMove missing AIController or Pawn. Target=%s"),
			*GetNameSafe(Target));
		return false;
	}

	UTsnEngagementSlotComponent* SlotComp =
		Target->FindComponentByClass<UTsnEngagementSlotComponent>();
	if (!SlotComp)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("MoveToEngagementBand: Target [%s] has no UTsnEngagementSlotComponent for Pawn=%s."),
			*GetNameSafe(Target),
			*GetNameSafe(AICon->GetPawn()));
		return false;
	}

	if (bForceRefreshSlotAssignment && SlotComp->HasSlot(AICon->GetPawn()))
	{
		SlotComp->ReleaseSlot(AICon->GetPawn());
	}

	// 槽位组件自身已经提供了“槽满时返回合理 fallback 位置”的容错，
	// 这里不要再提前 Failed，否则行为树会直接掉出接敌流程并长期 Idle。
	const bool bUsingFallbackSlotLocation = !SlotComp->IsSlotAvailable(AICon->GetPawn());
	if (bUsingFallbackSlotLocation)
	{
		UE_LOG(LogTireflySquadNav, Verbose,
			TEXT("MoveToEngagementBand: Target [%s] slots are full for Pawn=%s, using fallback location instead of failing."),
			*Target->GetName(),
			*GetNameSafe(AICon->GetPawn()));
	}

	CachedSlotPosition = SlotComp->RequestSlot(AICon->GetPawn(), CachedAttackRange);
	const FVector NavigationGoal = TsnMoveToEngagementBandInternal::ComputeEngagementNavigationGoal(
		AICon->GetPawn(),
		Target,
		CachedSlotPosition);
	const float BandRadius = FVector::Dist2D(Target->GetActorLocation(), CachedSlotPosition);
	const float EffectiveAcceptanceRadius = TsnMoveToEngagementBandInternal::ComputeEffectiveEngagementBandTolerance(
		AcceptanceRadius,
		CachedAttackRange,
		BandRadius);
	if (EffectiveAcceptanceRadius <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("MoveToEngagementBand: non-positive effective acceptance radius for Pawn=%s Target=%s Slot=%s BandRadius=%.1f AttackRange=%.1f BaseAcceptance=%.1f FallbackSlot=%s"),
			*GetNameSafe(AICon->GetPawn()),
			*GetNameSafe(Target),
			*CachedSlotPosition.ToCompactString(),
			BandRadius,
			CachedAttackRange,
			AcceptanceRadius,
			bUsingFallbackSlotLocation ? TEXT("true") : TEXT("false"));
	}

	FAIMoveRequest MoveReq;
	// 接战带语义仍然由目标周围的槽位环表达，
	// 但真正的导航目标需要向内收一个请求者胶囊半径，避免大胶囊长期停在环外。
	MoveReq.SetGoalLocation(NavigationGoal);
	MoveReq.SetAcceptanceRadius(EffectiveAcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetReachTestIncludesAgentRadius(false);

	const FPathFollowingRequestResult Result = AICon->MoveTo(MoveReq);
	CachedTargetLocation = Target->GetActorLocation();
	TimeSinceLastRePathCheck = 0.f;

	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("MoveToEngagementBand: MoveTo request failed for Pawn=%s Target=%s Slot=%s NavGoal=%s Acceptance=%.1f BandRadius=%.1f FallbackSlot=%s"),
			*GetNameSafe(AICon->GetPawn()),
			*GetNameSafe(Target),
			*CachedSlotPosition.ToCompactString(),
			*NavigationGoal.ToCompactString(),
			EffectiveAcceptanceRadius,
			BandRadius,
			bUsingFallbackSlotLocation ? TEXT("true") : TEXT("false"));
		SlotComp->ReleaseSlot(AICon->GetPawn());
		return false;
	}
	return true;
}

EBTNodeResult::Type UTsnBTTask_MoveToEngagementBand::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AActor* Target = GetTargetActor(OwnerComp);
	if (!AICon || !AICon->GetPawn() || !Target)
	{
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			Target,
			TEXT("ExecuteTask missing AIController, Pawn, or TargetActor"));
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn->Implements<UTsnTacticalUnit>())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_MoveToEngagementBand: Pawn [%s] does not implement ITsnTacticalUnit. Returning Failed."),
			*Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	CachedAttackRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
	if (CachedAttackRange <= 0.f)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_MoveToEngagementBand: GetEngagementRange() returned %.1f (must be > 0). Returning Failed."),
			CachedAttackRange);
		return EBTNodeResult::Failed;
	}

	if (UTsnEngagementSlotComponent* SlotComp = Target->FindComponentByClass<UTsnEngagementSlotComponent>())
	{
		FTsnEngagementSlotInfo AssignedSlotInfo;
		FVector AssignedSlotPosition = FVector::ZeroVector;
		if (SlotComp->TryGetAssignedSlotInfo(Pawn, AssignedSlotInfo, AssignedSlotPosition))
		{
			const float DistanceToTarget = FVector::Dist2D(Pawn->GetActorLocation(), Target->GetActorLocation());
			const float BandRadius = FVector::Dist2D(Target->GetActorLocation(), AssignedSlotPosition);
			const float EffectiveAcceptanceRadius = TsnMoveToEngagementBandInternal::ComputeEffectiveEngagementBandTolerance(
				AcceptanceRadius,
				CachedAttackRange,
				BandRadius);
			if (FMath::Abs(DistanceToTarget - BandRadius) <= EffectiveAcceptanceRadius)
			{
				return EBTNodeResult::Succeeded;
			}
		}
	}

	// 已在攻击距离内 → 直接进入后续攻击流程；若后续需要槽位，可由更高层组合决定是否先补做占位。
	if (FVector::Dist2D(Pawn->GetActorLocation(), Target->GetActorLocation()) <= CachedAttackRange)
	{
		return EBTNodeResult::Succeeded;
	}

	ElapsedApproachTime = 0.f;
	TimeSinceLastZeroIntentRecoveryCheck = 0.f;
	bIsFinishing = false;

	if (AActor* PreviousTarget = CachedTarget.Get())
	{
		UnbindTargetLifecycleDelegates(PreviousTarget);
	}
	CachedTarget.Reset();
	CachedOwnerComp.Reset();

	if (!RequestSlotAndMove(OwnerComp, Target))
	{
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			Target,
			TEXT("Initial RequestSlotAndMove returned false"));
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedTarget = Target;
	BindTargetLifecycleDelegates(Target);
	return EBTNodeResult::InProgress;
}

void UTsnBTTask_MoveToEngagementBand::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn())
	{
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			nullptr,
			TEXT("TickTask lost AIController or Pawn"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* Target = GetTargetActor(OwnerComp);
	if (!Target)
	{
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			nullptr,
			TEXT("TickTask lost TargetActor from blackboard"));
		ReleaseCurrentSlot(OwnerComp);
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector PawnLoc = AICon->GetPawn()->GetActorLocation();
	FVector LiveSlotPosition = CachedSlotPosition;
	if (UTsnEngagementSlotComponent* SlotComp = Target->FindComponentByClass<UTsnEngagementSlotComponent>())
	{
		FTsnEngagementSlotInfo AssignedSlotInfo;
		if (SlotComp->TryGetAssignedSlotInfo(AICon->GetPawn(), AssignedSlotInfo, LiveSlotPosition))
		{
			// Tick 阶段只读取当前已认领槽位的只读实时世界快照，
			// 避免为了“刷新当前位置”再次触发 RequestSlot 的写路径。
			CachedSlotPosition = LiveSlotPosition;
		}
	}

	const float DistanceToTarget = FVector::Dist2D(PawnLoc, Target->GetActorLocation());
	const float BandRadius = FVector::Dist2D(Target->GetActorLocation(), LiveSlotPosition);
	const float EffectiveAcceptanceRadius = TsnMoveToEngagementBandInternal::ComputeEffectiveEngagementBandTolerance(
		AcceptanceRadius,
		CachedAttackRange,
		BandRadius);
	const bool bWithinEngagementBand =
		FMath::Abs(DistanceToTarget - BandRadius) <= EffectiveAcceptanceRadius;

	if (bWithinEngagementBand)
	{
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (DistanceToTarget <= CachedAttackRange)
	{
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (UTsnTacticalMovementComponent* MoveComp =
			Cast<UTsnTacticalMovementComponent>(AICon->GetPawn()->GetMovementComponent()))
	{
		if (MoveComp->ConsumePendingEscapeRepathRequest())
		{
			UE_LOG(LogTireflySquadNav, Log,
				TEXT("MoveToEngagementBand: EscapeMode requested repath for Pawn=%s Target=%s"),
				*GetNameSafe(AICon->GetPawn()),
				*GetNameSafe(Target));
			if (!RequestSlotAndMove(OwnerComp, Target))
			{
				TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
					AICon,
					Target,
					TEXT("Escape-mode repath RequestSlotAndMove returned false"),
					DistanceToTarget,
					BandRadius,
					EffectiveAcceptanceRadius);
				AICon->StopMovement();
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
			TimeSinceLastZeroIntentRecoveryCheck = 0.f;
			return;
		}

		UPathFollowingComponent* PathComp = AICon->GetPathFollowingComponent();
		const FVector RequestedVelocity2D = FVector(
			MoveComp->GetLastUpdateRequestedVelocity().X,
			MoveComp->GetLastUpdateRequestedVelocity().Y,
			0.f);
		const float RequestedSpeed = RequestedVelocity2D.Size();
		const float ActualSpeed = AICon->GetPawn()->GetVelocity().Size2D();
		const bool bHasValidPath = PathComp && PathComp->HasValidPath();
		const bool bPathActivelyMoving = bHasValidPath
			&& AICon->GetMoveStatus() == EPathFollowingStatus::Moving;
		const bool bZeroIntentStall = !bPathActivelyMoving
			&& RequestedSpeed <= ZeroIntentRequestedSpeedThreshold
			&& ActualSpeed <= ZeroIntentActualSpeedThreshold;

		if (bZeroIntentStall)
		{
			TimeSinceLastZeroIntentRecoveryCheck += DeltaSeconds;
			if (TimeSinceLastZeroIntentRecoveryCheck >= ZeroIntentRecoveryDelay)
			{
				if (bWithinEngagementBand)
				{
					AICon->StopMovement();
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
					return;
				}

				const FString PathStatusDesc = PathComp
					? PathComp->GetStatusDesc()
					: TEXT("NoPathFollowingComponent");
				UE_LOG(LogTireflySquadNav, Warning,
					TEXT("MoveToEngagementBand: zero-intent stall recovery for Pawn=%s Target=%s DistToBand=%.1f RequestedSpeed=%.1f ActualSpeed=%.1f MoveStatus=%d PathStatus=%s HasValidPath=%s EscapeActive=%s"),
					*GetNameSafe(AICon->GetPawn()),
					*GetNameSafe(Target),
					FMath::Abs(DistanceToTarget - BandRadius),
					RequestedSpeed,
					ActualSpeed,
					static_cast<int32>(AICon->GetMoveStatus()),
					*PathStatusDesc,
					bHasValidPath ? TEXT("true") : TEXT("false"),
					MoveComp->IsEscapeModeActive() ? TEXT("true") : TEXT("false"));

				TimeSinceLastZeroIntentRecoveryCheck = 0.f;
				if (!RequestSlotAndMove(OwnerComp, Target, true))
				{
					TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
						AICon,
						Target,
						TEXT("Zero-intent recovery RequestSlotAndMove returned false"),
						DistanceToTarget,
						BandRadius,
						EffectiveAcceptanceRadius,
						RequestedSpeed,
						ActualSpeed);
					AICon->StopMovement();
					FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
					return;
				}
				return;
			}
		}
		else
		{
			TimeSinceLastZeroIntentRecoveryCheck = 0.f;
		}
	}
	else
	{
		TimeSinceLastZeroIntentRecoveryCheck = 0.f;
	}

	ElapsedApproachTime += DeltaSeconds;
	if (MaxApproachTime > 0.f && ElapsedApproachTime >= MaxApproachTime)
	{
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			Target,
			TEXT("MaxApproachTime exceeded"),
			DistanceToTarget,
			BandRadius,
			EffectiveAcceptanceRadius);
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	TimeSinceLastRePathCheck += DeltaSeconds;
	if (TimeSinceLastRePathCheck >= RePathCheckInterval)
	{
		TimeSinceLastRePathCheck = 0.f;
		const bool bSlotMoved =
			FVector::Dist2D(CachedSlotPosition, LiveSlotPosition) > RePathDistanceThreshold;
		const bool bPathStopped = AICon->GetMoveStatus() != EPathFollowingStatus::Moving;
		if (bPathStopped && bWithinEngagementBand)
		{
			AICon->StopMovement();
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		if (bSlotMoved || bPathStopped)
		{
			if (!RequestSlotAndMove(OwnerComp, Target, bPathStopped))
			{
				TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
					AICon,
					Target,
					bPathStopped
						? TEXT("Repath after path stopped returned false")
						: TEXT("Repath after slot moved returned false"),
					DistanceToTarget,
					BandRadius,
					EffectiveAcceptanceRadius);
				AICon->StopMovement();
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
			TimeSinceLastZeroIntentRecoveryCheck = 0.f;
		}
	}
}

EBTNodeResult::Type UTsnBTTask_MoveToEngagementBand::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AActor* BlackboardTarget = GetTargetActor(OwnerComp);
	UE_LOG(LogTireflySquadNav, Warning,
		TEXT("MoveToEngagementBand ABORTED: Pawn=%s BlackboardTarget=%s CachedTarget=%s MoveStatus=%d DistToCachedTarget=%.1f CachedAttackRange=%.1f CachedSlot=%s ElapsedApproach=%.2f"),
		*GetNameSafe(AICon ? AICon->GetPawn() : nullptr),
		*GetNameSafe(BlackboardTarget),
		*GetNameSafe(CachedTarget.Get()),
		AICon ? static_cast<int32>(AICon->GetMoveStatus()) : -1,
		(AICon && AICon->GetPawn() && CachedTarget.IsValid())
			? FVector::Dist2D(AICon->GetPawn()->GetActorLocation(), CachedTarget->GetActorLocation())
			: -1.f,
		CachedAttackRange,
		*CachedSlotPosition.ToCompactString(),
		ElapsedApproachTime);

	ReleaseCurrentSlot(OwnerComp);
	if (AICon)
	{
		AICon->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

void UTsnBTTask_MoveToEngagementBand::ReleaseCurrentSlot(
	UBehaviorTreeComponent& OwnerComp)
{
	AActor* Target = GetTargetActor(OwnerComp);
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!Target || !AICon || !AICon->GetPawn())
	{
		return;
	}

	if (UTsnEngagementSlotComponent* SlotComp =
			Target->FindComponentByClass<UTsnEngagementSlotComponent>())
	{
		SlotComp->ReleaseSlot(AICon->GetPawn());
	}
}

void UTsnBTTask_MoveToEngagementBand::BindTargetLifecycleDelegates(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	UnbindTargetLifecycleDelegates(Target);
	Target->OnDestroyed.AddDynamic(this, &UTsnBTTask_MoveToEngagementBand::OnTargetDestroyed);
	Target->OnEndPlay.AddDynamic(this, &UTsnBTTask_MoveToEngagementBand::OnTargetEndPlay);
}

void UTsnBTTask_MoveToEngagementBand::UnbindTargetLifecycleDelegates(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	Target->OnDestroyed.RemoveDynamic(this, &UTsnBTTask_MoveToEngagementBand::OnTargetDestroyed);
	Target->OnEndPlay.RemoveDynamic(this, &UTsnBTTask_MoveToEngagementBand::OnTargetEndPlay);
}

void UTsnBTTask_MoveToEngagementBand::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UE_LOG(LogTireflySquadNav, Verbose,
		TEXT("MoveToEngagementBand FINISHED: Result=%s Pawn=%s BlackboardTarget=%s CachedTarget=%s ElapsedApproach=%.2f CachedAttackRange=%.1f CachedSlot=%s"),
		TaskResult == EBTNodeResult::Succeeded
			? TEXT("Succeeded")
			: TaskResult == EBTNodeResult::Failed
				? TEXT("Failed")
				: TaskResult == EBTNodeResult::Aborted
					? TEXT("Aborted")
					: TEXT("InProgress"),
		*GetNameSafe(AICon ? AICon->GetPawn() : nullptr),
		*GetNameSafe(GetTargetActor(OwnerComp)),
		*GetNameSafe(CachedTarget.Get()),
		ElapsedApproachTime,
		CachedAttackRange,
		*CachedSlotPosition.ToCompactString());

	if (AActor* Target = CachedTarget.Get())
	{
		UnbindTargetLifecycleDelegates(Target);
	}
	CachedTarget.Reset();
	CachedOwnerComp.Reset();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UTsnBTTask_MoveToEngagementBand::OnTargetDestroyed(AActor* DestroyedActor)
{
	if (bIsFinishing)
	{
		return;
	}
	bIsFinishing = true;

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp)
	{
		return;
	}

	ReleaseCurrentSlot(*OwnerComp);
	if (AAIController* AICon = OwnerComp->GetAIOwner())
	{
		AICon->StopMovement();
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			DestroyedActor,
			TEXT("Target actor destroyed during MoveToEngagementBand"));
	}
	FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
}

void UTsnBTTask_MoveToEngagementBand::OnTargetEndPlay(
	AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (bIsFinishing)
	{
		return;
	}
	bIsFinishing = true;

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp)
	{
		return;
	}

	ReleaseCurrentSlot(*OwnerComp);
	if (AAIController* AICon = OwnerComp->GetAIOwner())
	{
		AICon->StopMovement();
		TsnMoveToEngagementBandInternal::LogMoveToEngagementBandFailure(
			AICon,
			Actor,
			TEXT("Target actor EndPlay triggered during MoveToEngagementBand"));
	}
	FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
}