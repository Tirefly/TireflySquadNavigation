// TsnBTDecorator_IsWithinEngagementBand.cpp

#include "BehaviorTree/TsnBTDecorator_IsWithinEngagementBand.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "TsnLog.h"

namespace TsnIsWithinEngagementBandInternal
{
	// 与 MoveToEngagementBand 的到位语义保持一致：
	// 允许很薄的一层几何抖动容差，但不能越过真实攻击距离窗口。
	constexpr float EngagementBandJitterTolerance = 5.f;
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
}

UTsnBTDecorator_IsWithinEngagementBand::UTsnBTDecorator_IsWithinEngagementBand()
{
	NodeName = TEXT("TSN Is Within Engagement Band");
}

void UTsnBTDecorator_IsWithinEngagementBand::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		if (TargetKey.IsNone())
		{
			UE_LOG(LogTireflySquadNav, Warning,
				TEXT("TsnBTDecorator_IsWithinEngagementBand: TargetKey '%s' not found in Blackboard '%s'; node will fail at runtime."),
				*TargetKey.SelectedKeyName.ToString(), *BBAsset->GetName());
		}
	}
	else
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTDecorator_IsWithinEngagementBand: TargetKey has no valid Blackboard asset; node will fail at runtime."));
	}
}

bool UTsnBTDecorator_IsWithinEngagementBand::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (!BB || !Pawn)
	{
		return false;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target)
	{
		return false;
	}

	if (!Pawn->Implements<UTsnTacticalUnit>())
	{
		return false;
	}

	const float AttackRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
	if (AttackRange <= 0.f)
	{
		return false;
	}

	UTsnEngagementSlotComponent* SlotComp = Target->FindComponentByClass<UTsnEngagementSlotComponent>();
	if (!SlotComp)
	{
		return false;
	}

	FTsnEngagementSlotInfo AssignedSlotInfo;
	FVector AssignedSlotPosition = FVector::ZeroVector;
	if (!SlotComp->TryGetAssignedSlotInfo(Pawn, AssignedSlotInfo, AssignedSlotPosition))
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist2D(Pawn->GetActorLocation(), Target->GetActorLocation());
	const float BandRadius = FVector::Dist2D(Target->GetActorLocation(), AssignedSlotPosition);
	const float EffectiveAcceptanceRadius = TsnIsWithinEngagementBandInternal::ComputeEffectiveEngagementBandTolerance(
		AcceptanceRadius,
		AttackRange,
		BandRadius);
	return FMath::Abs(DistanceToTarget - BandRadius) <= EffectiveAcceptanceRadius;
}