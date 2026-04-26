// TsnTestAIController.cpp
// 测试用 AI 控制器 —— CrowdFollowing 集成 + RVO 关闭

#include "TsnTestAIController.h"
#include "TsnTestLog.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ATsnTestAIController::ATsnTestAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(
		TEXT("PathFollowingComponent")))
{
}

void ATsnTestAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 关闭 CMC 内置 RVO Avoidance，防止与 DetourCrowd 双重避障冲突
	if (UCharacterMovementComponent* CMC = InPawn
		? InPawn->FindComponentByClass<UCharacterMovementComponent>() : nullptr)
	{
		CMC->bUseRVOAvoidance = false;
		UE_LOG(LogTsnTest, Log, TEXT("AIController [%s] possessed [%s], RVO disabled"),
			*GetName(), *InPawn->GetName());
	}
}
