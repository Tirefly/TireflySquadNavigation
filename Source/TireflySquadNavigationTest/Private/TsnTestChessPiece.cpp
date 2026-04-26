// TsnTestChessPiece.cpp
// 测试用 AI 棋子 —— ITsnTacticalUnit 实现 + 阶段可视化

#include "TsnTestChessPiece.h"
#include "TsnTestLog.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Components/TsnUnitSeparationComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

ATsnTestChessPiece::ATsnTestChessPiece(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTsnTacticalMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// 插件组件
	TacticalMovementComp = Cast<UTsnTacticalMovementComponent>(GetCharacterMovement());

	StanceObstacleComp = CreateDefaultSubobject<UTsnStanceObstacleComponent>(
		TEXT("StanceObstacleComp"));

	EngagementSlotComp = CreateDefaultSubobject<UTsnEngagementSlotComponent>(
		TEXT("EngagementSlotComp"));

	UnitSeparationComp = CreateDefaultSubobject<UTsnUnitSeparationComponent>(
		TEXT("UnitSeparationComp"));

	// 胶囊体可见
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetHiddenInGame(false);
		Capsule->SetVisibility(true);
		Capsule->ShapeColor = FColor::Red;
	}

	// AI Controller 默认类在蓝图或 AIControllerClass 中指定
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ATsnTestChessPiece::BeginPlay()
{
	Super::BeginPlay();

	// 根据 TeamID 设置胶囊体阵营颜色（一次性设置，不再改变）
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->ShapeColor = (TeamID == 1) ? FColor::Blue : FColor::Red;
	}

	UE_LOG(LogTsnTest, Log, TEXT("ChessPiece [%s] spawned, TeamID=%d, EngagementRange=%.0f"),
		*GetName(), TeamID, EngagementRange);
}

void ATsnTestChessPiece::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	if (!bDrawDebugSlotInfo) return;

	const FVector HeadLoc = GetActorLocation() + FVector(0.f, 0.f, 100.f);
	const FColor PhaseColor = GetPhaseDebugColor();
	const FString PhaseStr = GetPhaseDebugString();

	// 头顶阶段文字
	DrawDebugString(GetWorld(), HeadLoc, PhaseStr, nullptr, PhaseColor, 0.f, true);

	// 连线到目标 / 槽位
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBlackboardComponent())
	{
		AActor* Target = Cast<AActor>(
			AICon->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
		if (Target)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(),
				PhaseColor, false, 0.f, 0, 1.5f);
		}
	}
#endif
}

float ATsnTestChessPiece::GetEngagementRange_Implementation() const
{
	return EngagementRange;
}

bool ATsnTestChessPiece::IsInStanceMode_Implementation() const
{
	return StanceObstacleComp
		&& StanceObstacleComp->GetMobilityStance() == ETsnMobilityStance::Stance;
}

FString ATsnTestChessPiece::GetPhaseDebugString() const
{
	// 推断当前阶段
	const FString TypeStr = (EngagementRange <= 200.f) ? TEXT("Melee")
		: (EngagementRange <= 400.f) ? TEXT("Spear") : TEXT("Ranged");

	if (IsInStanceMode_Implementation())
	{
		return FString::Printf(TEXT("[%s] STANCE"), *TypeStr);
	}

	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBlackboardComponent())
	{
		AActor* Target = Cast<AActor>(
			AICon->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
		if (Target)
		{
			const float Dist = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
			const float PreEngagementDist = EngagementRange * 1.5f;

			if (Dist > PreEngagementDist)
			{
				return FString::Printf(TEXT("[%s] CHASE dist=%.0f"), *TypeStr, Dist);
			}
			else
			{
				return FString::Printf(TEXT("[%s] APPROACH dist=%.0f"), *TypeStr, Dist);
			}
		}
	}

	return FString::Printf(TEXT("[%s] IDLE"), *TypeStr);
}

FColor ATsnTestChessPiece::GetPhaseDebugColor() const
{
	if (IsInStanceMode_Implementation())
	{
		return FColor::Red;
	}

	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBlackboardComponent())
	{
		AActor* Target = Cast<AActor>(
			AICon->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
		if (Target)
		{
			const float Dist = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
			const float PreEngagementDist = EngagementRange * 1.5f;
			if (Dist > PreEngagementDist)
			{
				return FColor::Yellow;  // CHASE
			}
			return FColor::Cyan;  // APPROACH
		}
	}

	return FColor::White;  // IDLE
}
