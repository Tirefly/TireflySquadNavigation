// TsnBTTask_ChaseEngagementTarget.cpp

#include "BehaviorTree/TsnBTTask_ChaseEngagementTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "TsnLog.h"

namespace
{
	// 目标点必须明显落在预战斗半径内侧，否则 MoveTo 容易在“仍未进入 Chase 退出距离”时
	// 直接返回 AlreadyAtGoal，导致 PathFollowing 维持 Idle 且任务卡在 InProgress。
	constexpr float ChaseGoalRingInset = 90.f;
	constexpr float ChaseGoalAcceptanceRadius = 20.f;
	// 追击任务的成功判定需要比纯几何边界稍微宽一点，避免只差几厘米时
	// PathFollowing 已经停下，而 BT 还在等待“严格进入预战斗半径”。
	constexpr float ChaseArrivalJitterTolerance = 10.f;
	// 若导航投影把候选点吸回到 Pawn 身边或几乎不减少与目标的距离，
	// 这类追击点只会导致零意图恢复反复触发，应当直接丢弃。
	constexpr float ChaseGoalMinimumMoveDistance = 35.f;
	constexpr float ChaseGoalMinimumTargetDistanceGain = 10.f;
	constexpr float ChaseGoalProjectionExtentXY = 120.f;
	constexpr float ChaseGoalProjectionExtentZ = 200.f;
	constexpr float ChaseGoalAngleOffsets[] =
	{
		0.f,
		45.f,
		-45.f,
		90.f,
		-90.f,
		135.f,
		-135.f,
		180.f,
	};
}

UTsnBTTask_ChaseEngagementTarget::UTsnBTTask_ChaseEngagementTarget()
{
	NodeName = TEXT("TSN Chase Engagement Target");
	bNotifyTick = true;

	// 每个 BehaviorTreeComponent 创建独立实例，类成员变量不会跨 Agent 互相覆盖
	bCreateNodeInstance = true;
}

void UTsnBTTask_ChaseEngagementTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		if (TargetKey.IsNone())
		{
			UE_LOG(LogTireflySquadNav, Warning,
				TEXT("TsnBTTask_ChaseEngagementTarget: TargetKey '%s' not found in Blackboard '%s'; node will fail at runtime."),
				*TargetKey.SelectedKeyName.ToString(), *BBAsset->GetName());
		}
	}
	else
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_ChaseEngagementTarget: TargetKey has no valid Blackboard asset; node will fail at runtime."));
	}
}

AActor* UTsnBTTask_ChaseEngagementTarget::GetTargetActor(
	UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
}

bool UTsnBTTask_ChaseEngagementTarget::RequestChaseMove(
	UBehaviorTreeComponent& OwnerComp, AActor* Target)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (!AICon || !Pawn || !Target)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(
		AICon->GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_ChaseEngagementTarget: NavigationSystem missing for Pawn=%s Target=%s."),
			*GetNameSafe(Pawn),
			*GetNameSafe(Target));
		return false;
	}

	const FVector PawnLoc = Pawn->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();
	const float CurrentDistanceToTarget = FVector::Dist2D(PawnLoc, TargetLoc);
	FVector PreferredDirection = (PawnLoc - TargetLoc).GetSafeNormal2D();
	if (PreferredDirection.IsNearlyZero())
	{
		PreferredDirection = Target->GetActorForwardVector().GetSafeNormal2D();
		if (PreferredDirection.IsNearlyZero())
		{
			PreferredDirection = FVector::ForwardVector;
		}
	}

	const float DesiredGoalDistance = FMath::Max(
		CachedAttackRange,
		CachedPreEngagementRadius - ChaseGoalRingInset);
	const FVector ProjectionExtent(
		ChaseGoalProjectionExtentXY,
		ChaseGoalProjectionExtentXY,
		ChaseGoalProjectionExtentZ);

	FNavLocation PawnNavLocation;
	const bool bPawnProjected = NavSys->ProjectPointToNavigation(
		PawnLoc,
		PawnNavLocation,
		ProjectionExtent);

	FNavLocation ProjectedGoalLocation;
	FVector ChosenGoalLocation = FVector::ZeroVector;
	bool bFoundProjectedGoal = false;
	FVector FallbackProjectedGoalLocation = FVector::ZeroVector;
	bool bHasFallbackProjectedGoal = false;
	bool bRejectedProjectedGoal = false;
	FVector LastRejectedGoalLocation = FVector::ZeroVector;
	float LastRejectedGoalDistanceToTarget = 0.f;
	float LastRejectedGoalMoveDistance = 0.f;
	for (float AngleOffset : ChaseGoalAngleOffsets)
	{
		const FVector CandidateDirection = PreferredDirection.RotateAngleAxis(
			AngleOffset,
			FVector::UpVector).GetSafeNormal2D();
		const FVector CandidateGoal = TargetLoc + CandidateDirection * DesiredGoalDistance;
		if (NavSys->ProjectPointToNavigation(
				CandidateGoal,
				ProjectedGoalLocation,
				ProjectionExtent))
		{
			if (!bHasFallbackProjectedGoal)
			{
				FallbackProjectedGoalLocation = ProjectedGoalLocation.Location;
				bHasFallbackProjectedGoal = true;
			}

			const float CandidateDistanceToTarget = FVector::Dist2D(
				ProjectedGoalLocation.Location,
				TargetLoc);
			const float CandidateMoveDistance = FVector::Dist2D(
				ProjectedGoalLocation.Location,
				PawnLoc);
			const bool bGoalStillOutsidePreEngagement = CandidateDistanceToTarget
				> CachedPreEngagementRadius - ChaseArrivalJitterTolerance;
			const bool bGoalMakesTooLittleProgress = CurrentDistanceToTarget
				> CachedPreEngagementRadius + ChaseArrivalJitterTolerance
				&& (CandidateMoveDistance <= ChaseGoalMinimumMoveDistance
					|| CandidateDistanceToTarget >= CurrentDistanceToTarget - ChaseGoalMinimumTargetDistanceGain);
			if (bGoalStillOutsidePreEngagement || bGoalMakesTooLittleProgress)
			{
				bRejectedProjectedGoal = true;
				LastRejectedGoalLocation = ProjectedGoalLocation.Location;
				LastRejectedGoalDistanceToTarget = CandidateDistanceToTarget;
				LastRejectedGoalMoveDistance = CandidateMoveDistance;
				continue;
			}

			ChosenGoalLocation = ProjectedGoalLocation.Location;
			bFoundProjectedGoal = true;
			break;
		}
	}

	if (!bFoundProjectedGoal)
	{
		if (bHasFallbackProjectedGoal)
		{
			ChosenGoalLocation = FallbackProjectedGoalLocation;
			bFoundProjectedGoal = true;
			UE_LOG(LogTireflySquadNav, Verbose,
				TEXT("TsnBTTask_ChaseEngagementTarget: falling back to first projected chase goal for Pawn=%s Target=%s Goal=%s RejectedProjectedGoal=%s RejectedGoal=%s RejectedGoalDist=%.1f RejectedGoalMove=%.1f"),
				*GetNameSafe(Pawn),
				*GetNameSafe(Target),
				*ChosenGoalLocation.ToCompactString(),
				bRejectedProjectedGoal ? TEXT("true") : TEXT("false"),
				*LastRejectedGoalLocation.ToCompactString(),
				LastRejectedGoalDistanceToTarget,
				LastRejectedGoalMoveDistance);
		}
	}

	if (!bFoundProjectedGoal)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_ChaseEngagementTarget: failed to find useful chase goal for Pawn=%s Target=%s PawnProjected=%s PawnLoc=%s TargetLoc=%s DesiredGoalDistance=%.1f RejectedProjectedGoal=%s RejectedGoal=%s RejectedGoalDist=%.1f RejectedGoalMove=%.1f"),
			*GetNameSafe(Pawn),
			*GetNameSafe(Target),
			bPawnProjected ? TEXT("true") : TEXT("false"),
			*PawnLoc.ToCompactString(),
			*TargetLoc.ToCompactString(),
			DesiredGoalDistance,
			bRejectedProjectedGoal ? TEXT("true") : TEXT("false"),
			*LastRejectedGoalLocation.ToCompactString(),
			LastRejectedGoalDistanceToTarget,
			LastRejectedGoalMoveDistance);
		return false;
	}

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(ChosenGoalLocation);
	// 任务完成条件由 TickTask 中的“到目标的二维距离 <= CachedPreEngagementRadius”控制，
	// 因此 MoveTo 这里只需要一个较小的局部到点容差，避免过早 AlreadyAtGoal。
	MoveReq.SetAcceptanceRadius(ChaseGoalAcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetAllowPartialPath(true);
	MoveReq.SetProjectGoalLocation(false);
	// 与本节点的 Dist2D 判定保持一致：预战斗半径按中心点距离计算，
	// 不再额外叠加追击者胶囊半径，避免 PathFollowing 已经停下
	// 但 TickTask 仍认为“尚未进入预战斗距离”而永久卡在 InProgress。
	MoveReq.SetReachTestIncludesAgentRadius(false);

	const FPathFollowingRequestResult Result = AICon->MoveTo(MoveReq);
	UPathFollowingComponent* PathComp = AICon->GetPathFollowingComponent();
	const bool bHasValidPath = PathComp && PathComp->HasValidPath();
	if (Result.Code == EPathFollowingRequestResult::Failed || !bHasValidPath)
	{
		const FString PathStatusDesc = PathComp
			? PathComp->GetStatusDesc()
			: TEXT("NoPathFollowingComponent");
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_ChaseEngagementTarget: chase move request suspicious for Pawn=%s Target=%s Result=%d MoveStatus=%d PathStatus=%s HasValidPath=%s PawnProjected=%s Goal=%s TargetLoc=%s"),
			*GetNameSafe(Pawn),
			*GetNameSafe(Target),
			static_cast<int32>(Result.Code),
			static_cast<int32>(AICon->GetMoveStatus()),
			*PathStatusDesc,
			bHasValidPath ? TEXT("true") : TEXT("false"),
			bPawnProjected ? TEXT("true") : TEXT("false"),
			*ChosenGoalLocation.ToCompactString(),
			*TargetLoc.ToCompactString());
	}

	return Result.Code != EPathFollowingRequestResult::Failed;
}

EBTNodeResult::Type UTsnBTTask_ChaseEngagementTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn()) return EBTNodeResult::Failed;

	AActor* Target = GetTargetActor(OwnerComp);
	if (!Target) return EBTNodeResult::Failed;

	// 从接口读取攻击距离——未实现接口或返回非法值时直接 Failed
	APawn* Pawn = AICon->GetPawn();
	if (!Pawn->Implements<UTsnTacticalUnit>())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_ChaseEngagementTarget: Pawn [%s] does not implement ITsnTacticalUnit. Returning Failed."),
			*Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	CachedAttackRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
	if (CachedAttackRange <= 0.f)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_ChaseEngagementTarget: GetEngagementRange() returned %.1f (must be > 0). Returning Failed."),
			CachedAttackRange);
		return EBTNodeResult::Failed;
	}
	CachedPreEngagementRadius = CachedAttackRange * PreEngagementRadiusMultiplier;

	// 已在预战斗距离内（含已在攻击距离内）→ 无需追击
	if (FVector::Dist2D(Pawn->GetActorLocation(), Target->GetActorLocation())
		<= CachedPreEngagementRadius + ChaseArrivalJitterTolerance)
	{
		return EBTNodeResult::Succeeded;
	}
	TimeSinceLastPathRecoveryCheck = 0.f;
	TimeSinceLastZeroIntentRecoveryCheck = 0.f;

	if (!RequestChaseMove(OwnerComp, Target))
		return EBTNodeResult::Failed;

	return EBTNodeResult::InProgress;
}

void UTsnBTTask_ChaseEngagementTarget::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* Target = GetTargetActor(OwnerComp);
	if (!Target)
	{
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	float Dist = FVector::Dist2D(AICon->GetPawn()->GetActorLocation(),
		Target->GetActorLocation());
	if (Dist <= CachedPreEngagementRadius + ChaseArrivalJitterTolerance)
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
				TEXT("ChaseEngagementTarget: EscapeMode requested repath for Pawn=%s Target=%s"),
				*GetNameSafe(AICon->GetPawn()),
				*GetNameSafe(Target));
			if (!RequestChaseMove(OwnerComp, Target))
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
			TimeSinceLastPathRecoveryCheck = 0.f;
			TimeSinceLastZeroIntentRecoveryCheck = 0.f;
		}

		UPathFollowingComponent* PathComp = AICon->GetPathFollowingComponent();
		const FVector RequestedVelocity2D = FVector(
			MoveComp->GetLastUpdateRequestedVelocity().X,
			MoveComp->GetLastUpdateRequestedVelocity().Y,
			0.f);
		const float RequestedSpeed = RequestedVelocity2D.Size();
		const float ActualSpeed = AICon->GetPawn()->GetVelocity().Size2D();
		const bool bZeroIntentStall = RequestedSpeed <= ZeroIntentRequestedSpeedThreshold
			&& ActualSpeed <= ZeroIntentActualSpeedThreshold;

		if (bZeroIntentStall)
		{
			TimeSinceLastZeroIntentRecoveryCheck += DeltaSeconds;
			if (TimeSinceLastZeroIntentRecoveryCheck >= ZeroIntentRecoveryDelay)
			{
				const FString PathStatusDesc = PathComp
					? PathComp->GetStatusDesc()
					: TEXT("NoPathFollowingComponent");
				const bool bHasValidPath = PathComp && PathComp->HasValidPath();
				UE_LOG(LogTireflySquadNav, Warning,
					TEXT("ChaseEngagementTarget: zero-intent stall recovery for Pawn=%s Target=%s Dist=%.1f RequestedSpeed=%.1f ActualSpeed=%.1f MoveStatus=%d PathStatus=%s HasValidPath=%s EscapeActive=%s"),
					*GetNameSafe(AICon->GetPawn()),
					*GetNameSafe(Target),
					Dist,
					RequestedSpeed,
					ActualSpeed,
					static_cast<int32>(AICon->GetMoveStatus()),
					*PathStatusDesc,
					bHasValidPath ? TEXT("true") : TEXT("false"),
					MoveComp->IsEscapeModeActive() ? TEXT("true") : TEXT("false"));

				TimeSinceLastZeroIntentRecoveryCheck = 0.f;
				if (!bHasValidPath && AICon->GetMoveStatus() != EPathFollowingStatus::Moving)
				{
					AICon->StopMovement();
					FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
					return;
				}

				if (!RequestChaseMove(OwnerComp, Target))
				{
					FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
					return;
				}
				TimeSinceLastPathRecoveryCheck = 0.f;
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

	TimeSinceLastPathRecoveryCheck += DeltaSeconds;
	if (TimeSinceLastPathRecoveryCheck >= PathRecoveryInterval)
	{
		TimeSinceLastPathRecoveryCheck = 0.f;
		if (AICon->GetMoveStatus() != EPathFollowingStatus::Moving)
		{
			if (!RequestChaseMove(OwnerComp, Target))
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
			TimeSinceLastZeroIntentRecoveryCheck = 0.f;
		}
	}
}

EBTNodeResult::Type UTsnBTTask_ChaseEngagementTarget::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner()) AICon->StopMovement();
	return EBTNodeResult::Aborted;
}
