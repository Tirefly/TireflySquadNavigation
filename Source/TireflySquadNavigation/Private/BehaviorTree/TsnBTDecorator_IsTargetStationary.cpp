// TsnBTDecorator_IsTargetStationary.cpp

#include "BehaviorTree/TsnBTDecorator_IsTargetStationary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TsnLog.h"

UTsnBTDecorator_IsTargetStationary::UTsnBTDecorator_IsTargetStationary()
{
	NodeName = TEXT("TSN Is Target Stationary");
}

void UTsnBTDecorator_IsTargetStationary::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		if (TargetKey.IsNone())
		{
			UE_LOG(LogTireflySquadNav, Warning,
				TEXT("TsnBTDecorator_IsTargetStationary: TargetKey '%s' not found in Blackboard '%s'; node will fail at runtime."),
				*TargetKey.SelectedKeyName.ToString(), *BBAsset->GetName());
		}
	}
	else
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnBTDecorator_IsTargetStationary: TargetKey has no valid Blackboard asset; node will fail at runtime."));
	}
}

bool UTsnBTDecorator_IsTargetStationary::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target) return false;

	if (ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		if (UCharacterMovementComponent* MoveComp = TargetChar->GetCharacterMovement())
		{
			return MoveComp->Velocity.Size2D() < SpeedThreshold;
		}
	}
	return Target->GetVelocity().Size2D() < SpeedThreshold;
}
