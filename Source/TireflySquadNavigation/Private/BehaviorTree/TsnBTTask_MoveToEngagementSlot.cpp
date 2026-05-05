// TsnBTTask_MoveToEngagementSlot.cpp

#include "BehaviorTree/TsnBTTask_MoveToEngagementSlot.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "TsnLog.h"

namespace
{
	// Crowd/PathFollowing 在接近槽位时可能会停在 AcceptanceRadius 外几个单位内，
	// 给一个很小的抖动容差，避免任务在 30.1 / 30.5 这类边缘距离上反复零意图恢复。
	constexpr float SlotArrivalJitterTolerance = 5.f;
}

UTsnBTTask_MoveToEngagementSlot::UTsnBTTask_MoveToEngagementSlot()
{
	NodeName = TEXT("TSN Move To Engagement Slot");
	bNotifyTick = true;

	// 每个 BehaviorTreeComponent 创建独立实例，类成员变量（含委托绑定）不会跨 Agent 互相覆盖
	bCreateNodeInstance = true;
}

void UTsnBTTask_MoveToEngagementSlot::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		if (TargetKey.IsNone())
		{
			UE_LOG(LogTireflySquadNav, Warning,
				TEXT("TsnBTTask_MoveToEngagementSlot: TargetKey '%s' not found in Blackboard '%s'; node will fail at runtime."),
				*TargetKey.SelectedKeyName.ToString(), *BBAsset->GetName());
		}
	}
	else
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_MoveToEngagementSlot: TargetKey has no valid Blackboard asset; node will fail at runtime."));
	}
}

AActor* UTsnBTTask_MoveToEngagementSlot::GetTargetActor(
	UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
}

bool UTsnBTTask_MoveToEngagementSlot::RequestSlotAndMove(
	UBehaviorTreeComponent& OwnerComp, AActor* Target, bool bForceRefreshSlotAssignment)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn()) return false;

	UTsnEngagementSlotComponent* SlotComp =
		Target->FindComponentByClass<UTsnEngagementSlotComponent>();
	if (!SlotComp) return false;

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
			TEXT("MoveToEngagementSlot: Target [%s] slots are full for Pawn=%s, using fallback location instead of failing."),
			*Target->GetName(),
			*GetNameSafe(AICon->GetPawn()));
	}

	CachedSlotPosition = SlotComp->RequestSlot(AICon->GetPawn(), CachedAttackRange);
	const float EffectiveAcceptanceRadius = AcceptanceRadius + SlotArrivalJitterTolerance;

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(CachedSlotPosition);
	MoveReq.SetAcceptanceRadius(EffectiveAcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	// 与 TickTask 的槽位到达判定保持一致：按中心点到槽位快照的二维距离判断，
	// 不额外叠加追击者胶囊半径，避免 PathFollowing 先停下而任务仍卡在 InProgress。
	MoveReq.SetReachTestIncludesAgentRadius(false);

	FPathFollowingRequestResult Result = AICon->MoveTo(MoveReq);
	CachedTargetLocation = Target->GetActorLocation();
	TimeSinceLastRePathCheck = 0.f;

	// MoveTo 失败时释放已分配的槽位，防止泄漏
	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		SlotComp->ReleaseSlot(AICon->GetPawn());
		return false;
	}
	return true;
}

EBTNodeResult::Type UTsnBTTask_MoveToEngagementSlot::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AActor* Target = GetTargetActor(OwnerComp);
	if (!AICon || !AICon->GetPawn() || !Target) return EBTNodeResult::Failed;

	APawn* Pawn = AICon->GetPawn();

	// 接口合法性检查：未实现接口或返回非法值时直接 Failed
	if (!Pawn->Implements<UTsnTacticalUnit>())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_MoveToEngagementSlot: Pawn [%s] does not implement ITsnTacticalUnit. Returning Failed."),
			*Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	CachedAttackRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
	if (CachedAttackRange <= 0.f)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTTask_MoveToEngagementSlot: GetEngagementRange() returned %.1f (must be > 0). Returning Failed."),
			CachedAttackRange);
		return EBTNodeResult::Failed;
	}

	// 已在攻击距离内 → 直接进入站桩攻击，不分配槽位
	if (FVector::Dist2D(Pawn->GetActorLocation(), Target->GetActorLocation())
		<= CachedAttackRange)
		return EBTNodeResult::Succeeded;

	ElapsedApproachTime = 0.f;
	TimeSinceLastZeroIntentRecoveryCheck = 0.f;
	bIsFinishing = false;     // 每次任务启动时重置，防止上次执行的残留状态干扰

	// 清理上次执行遗留的目标委托，避免同一任务实例重复绑定触发 ensure。
	if (AActor* PreviousTarget = CachedTarget.Get())
	{
		UnbindTargetLifecycleDelegates(PreviousTarget);
	}
	CachedTarget.Reset();
	CachedOwnerComp.Reset();

	if (!RequestSlotAndMove(OwnerComp, Target))
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedTarget = Target;
	BindTargetLifecycleDelegates(Target);
	return EBTNodeResult::InProgress;
}

void UTsnBTTask_MoveToEngagementSlot::TickTask(
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
		ReleaseCurrentSlot(OwnerComp);
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector PawnLoc = AICon->GetPawn()->GetActorLocation();
	const float EffectiveAcceptanceRadius = AcceptanceRadius + SlotArrivalJitterTolerance;
	FVector LiveSlotPosition = CachedSlotPosition;
	if (UTsnEngagementSlotComponent* SlotComp = Target->FindComponentByClass<UTsnEngagementSlotComponent>())
	{
		// 使用当前已认领槽位的实时世界快照做判定，避免长期追逐过期的旧位置。
		LiveSlotPosition = SlotComp->RequestSlot(AICon->GetPawn(), CachedAttackRange);
	}
	const float DistanceToTarget = FVector::Dist2D(PawnLoc, Target->GetActorLocation());
	const float LiveSlotRadius = FVector::Dist2D(Target->GetActorLocation(), LiveSlotPosition);
	const bool bWithinSlotRadiusTolerance = FMath::Abs(DistanceToTarget - LiveSlotRadius) <= EffectiveAcceptanceRadius;

	// 退出条件 (a)：到达槽位
	if (FVector::Dist2D(PawnLoc, LiveSlotPosition) <= EffectiveAcceptanceRadius)
	{
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 退出条件 (b)：中途进入攻击距离（目标向自身靠近）
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
				TEXT("MoveToEngagementSlot: EscapeMode requested repath for Pawn=%s Target=%s"),
				*GetNameSafe(AICon->GetPawn()),
				*GetNameSafe(Target));
			if (!RequestSlotAndMove(OwnerComp, Target))
			{
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
		const bool bZeroIntentStall = RequestedSpeed <= ZeroIntentRequestedSpeedThreshold
			&& ActualSpeed <= ZeroIntentActualSpeedThreshold;

		if (bZeroIntentStall)
		{
			TimeSinceLastZeroIntentRecoveryCheck += DeltaSeconds;
			if (TimeSinceLastZeroIntentRecoveryCheck >= ZeroIntentRecoveryDelay)
			{
				if (bWithinSlotRadiusTolerance)
				{
					// 已经处在目标周围的正确半径带里时，
					// 不要继续为了精确角度与排斥/分离力对抗，直接交给后续站桩战斗阶段。
					AICon->StopMovement();
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
					return;
				}

				const FString PathStatusDesc = PathComp
					? PathComp->GetStatusDesc()
					: TEXT("NoPathFollowingComponent");
				const bool bHasValidPath = PathComp && PathComp->HasValidPath();
				UE_LOG(LogTireflySquadNav, Warning,
					TEXT("MoveToEngagementSlot: zero-intent stall recovery for Pawn=%s Target=%s DistToSlot=%.1f RequestedSpeed=%.1f ActualSpeed=%.1f MoveStatus=%d PathStatus=%s HasValidPath=%s EscapeActive=%s"),
					*GetNameSafe(AICon->GetPawn()),
					*GetNameSafe(Target),
					FVector::Dist2D(PawnLoc, LiveSlotPosition),
					RequestedSpeed,
					ActualSpeed,
					static_cast<int32>(AICon->GetMoveStatus()),
					*PathStatusDesc,
					bHasValidPath ? TEXT("true") : TEXT("false"),
					MoveComp->IsEscapeModeActive() ? TEXT("true") : TEXT("false"));

				TimeSinceLastZeroIntentRecoveryCheck = 0.f;
				// 零意图恢复时不要死守旧槽位；释放后重认领，避免反复重试坏角度。
				if (!RequestSlotAndMove(OwnerComp, Target, true))
				{
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

	// 退出条件 (c)：超时安全兜底（路径异常兜底）
	ElapsedApproachTime += DeltaSeconds;
	if (MaxApproachTime > 0.f && ElapsedApproachTime >= MaxApproachTime)
	{
		AICon->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 周期性检查：若目标已明显移动，则基于已持有槽位的本地参数刷新世界快照并重发 MoveTo
	TimeSinceLastRePathCheck += DeltaSeconds;
	if (TimeSinceLastRePathCheck >= RePathCheckInterval)
	{
		TimeSinceLastRePathCheck = 0.f;
		const bool bSlotMoved =
			FVector::Dist2D(CachedSlotPosition, LiveSlotPosition)
			> RePathDistanceThreshold;
		const bool bPathStopped = AICon->GetMoveStatus() != EPathFollowingStatus::Moving;
		if (bPathStopped && bWithinSlotRadiusTolerance)
		{
			AICon->StopMovement();
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		if (bSlotMoved || bPathStopped)
		{
			if (!RequestSlotAndMove(OwnerComp, Target, bPathStopped))
			{
				AICon->StopMovement();
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
			TimeSinceLastZeroIntentRecoveryCheck = 0.f;
		}
	}
}

EBTNodeResult::Type UTsnBTTask_MoveToEngagementSlot::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ReleaseCurrentSlot(OwnerComp);
	if (AAIController* AICon = OwnerComp.GetAIOwner()) AICon->StopMovement();
	return EBTNodeResult::Aborted;
}

void UTsnBTTask_MoveToEngagementSlot::ReleaseCurrentSlot(
	UBehaviorTreeComponent& OwnerComp)
{
	AActor* Target = GetTargetActor(OwnerComp);
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!Target || !AICon || !AICon->GetPawn()) return;

	if (UTsnEngagementSlotComponent* SlotComp =
			Target->FindComponentByClass<UTsnEngagementSlotComponent>())
	{
		SlotComp->ReleaseSlot(AICon->GetPawn());
	}
}

void UTsnBTTask_MoveToEngagementSlot::BindTargetLifecycleDelegates(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	UnbindTargetLifecycleDelegates(Target);
	Target->OnDestroyed.AddDynamic(this, &UTsnBTTask_MoveToEngagementSlot::OnTargetDestroyed);
	Target->OnEndPlay.AddDynamic(this, &UTsnBTTask_MoveToEngagementSlot::OnTargetEndPlay);
}

void UTsnBTTask_MoveToEngagementSlot::UnbindTargetLifecycleDelegates(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	Target->OnDestroyed.RemoveDynamic(this, &UTsnBTTask_MoveToEngagementSlot::OnTargetDestroyed);
	Target->OnEndPlay.RemoveDynamic(this, &UTsnBTTask_MoveToEngagementSlot::OnTargetEndPlay);
}

void UTsnBTTask_MoveToEngagementSlot::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	// 使用缓存的目标指针解绑委托，而非从 Blackboard 读取——
	// 目标被销毁时 BB 可能已清除引用，GetTargetActor 会返回 nullptr 导致解绑被跳过。
	if (AActor* Target = CachedTarget.Get())
	{
		UnbindTargetLifecycleDelegates(Target);
	}
	CachedTarget.Reset();
	CachedOwnerComp.Reset();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UTsnBTTask_MoveToEngagementSlot::OnTargetDestroyed(AActor* DestroyedActor)
{
	// Actor 销毁时 OnEndPlay 必先于 OnDestroyed 触发，bIsFinishing 已被置 true，直接 return
	if (bIsFinishing) return;
	bIsFinishing = true;

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp) return;

	ReleaseCurrentSlot(*OwnerComp);
	if (AAIController* AICon = OwnerComp->GetAIOwner())
	{
		AICon->StopMovement();
	}
	FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
}

void UTsnBTTask_MoveToEngagementSlot::OnTargetEndPlay(
	AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	// 防双触发：OnEndPlay 可能触发两次（对象池回收 + 真正销毁），或与 OnDestroyed 先后触发
	if (bIsFinishing) return;
	bIsFinishing = true;

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp) return;

	ReleaseCurrentSlot(*OwnerComp);
	if (AAIController* AICon = OwnerComp->GetAIOwner())
	{
		AICon->StopMovement();
	}
	FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
}
