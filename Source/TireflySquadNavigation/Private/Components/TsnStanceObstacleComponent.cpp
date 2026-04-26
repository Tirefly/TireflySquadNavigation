// TsnStanceObstacleComponent.cpp

#include "Components/TsnStanceObstacleComponent.h"
#include "NavModifierComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavAreas/NavArea_Null.h"
#include "NavAreas/TsnNavArea_StanceUnit.h"
#include "Subsystems/TsnStanceRepulsionSubsystem.h"
#include "TsnLog.h"
#include "DrawDebugHelpers.h"

#if ENABLE_DRAW_DEBUG
static bool GDrawDebugTsnStanceObstacle = false;
static FAutoConsoleVariableRef CVarDrawDebugTsnStanceObstacle(
	TEXT("tsn.debug.DrawStanceObstacle"),
	GDrawDebugTsnStanceObstacle,
	TEXT("1 = Draw TsnStanceObstacleComponent obstacle radius, nav modifier radius and state labels (Development/Debug only)"),
	ECVF_Default);
#endif

UTsnStanceObstacleComponent::UTsnStanceObstacleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTsnStanceObstacleComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheComponents();

	if (bUseNavModifier)
	{
		InitNavModifier();
	}
}

void UTsnStanceObstacleComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugObstacle || GDrawDebugTsnStanceObstacle)
	{
		DrawDebugObstacleState();
	}
#endif
}

void UTsnStanceObstacleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(NavModifierDeactivationTimer);

	if (CurrentMobilityStance == ETsnMobilityStance::Stance)
	{
		if (UTsnStanceRepulsionSubsystem* RepSys =
				GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
		{
			RepSys->UnregisterStanceUnit(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UTsnStanceObstacleComponent::CacheComponents()
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* AICon = Cast<AAIController>(OwnerPawn->GetController()))
		{
			CrowdFollowingComp = Cast<UCrowdFollowingComponent>(
				AICon->GetPathFollowingComponent());
		}
	}
}

void UTsnStanceObstacleComponent::GetSanitizedRadii(
	float& OutObstacleRadius, float& OutRepulsionRadius) const
{
	OutObstacleRadius = FMath::Max(ObstacleRadius, 10.f);

	const bool bValidOuterBand = RepulsionRadius > OutObstacleRadius;
	ensureMsgf(bValidOuterBand,
		TEXT("TsnStanceObstacleComponent: RepulsionRadius (%.1f) must be > ObstacleRadius (%.1f)."),
		RepulsionRadius, OutObstacleRadius);

	OutRepulsionRadius = bValidOuterBand
		? RepulsionRadius
		: (OutObstacleRadius + 30.f);
}

float UTsnStanceObstacleComponent::GetEffectiveNavModifierRadius(
	float SanitizedObstacleRadius) const
{
	return SanitizedObstacleRadius + FMath::Max(NavModifierExtraRadius, 0.f);
}

void UTsnStanceObstacleComponent::InitNavModifier()
{
	float EffectiveObstacleRadius = 0.f;
	float IgnoredRepulsionRadius = 0.f;
	GetSanitizedRadii(EffectiveObstacleRadius, IgnoredRepulsionRadius);
	const float EffectiveNavModifierRadius =
		GetEffectiveNavModifierRadius(EffectiveObstacleRadius);

	// 开关式：BeginPlay 预创建，初始状态关闭
	NavModifierComp = NewObject<UNavModifierComponent>(
		GetOwner(), UNavModifierComponent::StaticClass(),
		TEXT("TsnStanceObstacleNavMod"));

	NavModifierComp->FailsafeExtent =
		FVector(EffectiveNavModifierRadius, EffectiveNavModifierRadius, 100.f);
	ApplyNavAreaClass();
	NavModifierComp->SetNavigationRelevancy(false);
	NavModifierComp->RegisterComponent();
}

void UTsnStanceObstacleComponent::ApplyNavAreaClass()
{
	if (!NavModifierComp) return;

	switch (NavModifierMode)
	{
	case ETsnNavModifierMode::Impassable:
		NavModifierComp->SetAreaClass(UNavArea_Null::StaticClass());
		break;
	case ETsnNavModifierMode::HighCost:
		NavModifierComp->SetAreaClass(UTsnNavArea_StanceUnit::StaticClass());
		break;
	}
}

void UTsnStanceObstacleComponent::ActivateNavModifier()
{
	if (!NavModifierComp) return;

	GetWorld()->GetTimerManager().ClearTimer(NavModifierDeactivationTimer);
	NavModifierComp->UpdateNavigationBounds();
	NavModifierComp->SetNavigationRelevancy(true);
}

void UTsnStanceObstacleComponent::DeactivateNavModifier()
{
	if (!NavModifierComp) return;

	NavModifierComp->SetNavigationRelevancy(false);
}

void UTsnStanceObstacleComponent::EnterStanceMode()
{
	if (CurrentMobilityStance == ETsnMobilityStance::Stance) return;
	CurrentMobilityStance = ETsnMobilityStance::Stance;

	// 补缓存（BeginPlay 时 Controller 可能还未 Possess）
	if (!CrowdFollowingComp) CacheComponents();

	if (CrowdFollowingComp)
		CrowdFollowingComp->SetCrowdSimulationState(ECrowdSimulationState::ObstacleOnly);

	if (bUseNavModifier)
		ActivateNavModifier();

	float EffectiveObstacleRadius = 0.f;
	float EffectiveRepulsionRadius = 0.f;
	GetSanitizedRadii(EffectiveObstacleRadius, EffectiveRepulsionRadius);

	if (UTsnStanceRepulsionSubsystem* RepSys =
			GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
	{
		RepSys->RegisterStanceUnit(
			GetOwner(), EffectiveRepulsionRadius, RepulsionStrength, EffectiveObstacleRadius);
	}
}

void UTsnStanceObstacleComponent::ExitStanceMode()
{
	if (CurrentMobilityStance == ETsnMobilityStance::Moving) return;
	CurrentMobilityStance = ETsnMobilityStance::Moving;

	if (CrowdFollowingComp)
		CrowdFollowingComp->SetCrowdSimulationState(ECrowdSimulationState::Enabled);

	// 延迟关闭：避免其他单位路径突然跳变；对象池回收时立即关闭
	if (bUseNavModifier && NavModifierComp)
	{
		if (bIsReleasingOwner)
		{
			DeactivateNavModifier();
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(
				NavModifierDeactivationTimer, this,
				&UTsnStanceObstacleComponent::DeactivateNavModifier,
				NavModifierDeactivationDelay, false);
		}
	}

	if (UTsnStanceRepulsionSubsystem* RepSys =
			GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
	{
		RepSys->UnregisterStanceUnit(GetOwner());
	}
}

void UTsnStanceObstacleComponent::UpdateStanceUnitParams()
{
	float EffectiveObstacleRadius = 0.f;
	float EffectiveRepulsionRadius = 0.f;
	GetSanitizedRadii(EffectiveObstacleRadius, EffectiveRepulsionRadius);
	const float EffectiveNavModifierRadius =
		GetEffectiveNavModifierRadius(EffectiveObstacleRadius);

	if (bUseNavModifier && NavModifierComp)
	{
		NavModifierComp->FailsafeExtent =
			FVector(EffectiveNavModifierRadius, EffectiveNavModifierRadius, 100.f);
		NavModifierComp->UpdateNavigationBounds();

		if (UNavigationSystemV1* NavSys =
				FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			NavSys->UpdateComponentInNavOctree(*NavModifierComp);
		}
	}

	if (CurrentMobilityStance == ETsnMobilityStance::Stance)
	{
		if (UTsnStanceRepulsionSubsystem* RepSys =
				GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
		{
			RepSys->UpdateStanceUnit(
				GetOwner(), EffectiveRepulsionRadius, RepulsionStrength, EffectiveObstacleRadius);
		}
	}
}

void UTsnStanceObstacleComponent::OnOwnerReleased()
{
	bIsReleasingOwner = true;

	if (CurrentMobilityStance == ETsnMobilityStance::Stance)
	{
		ExitStanceMode();
	}

	// 兜底：清理残留定时器
	GetWorld()->GetTimerManager().ClearTimer(NavModifierDeactivationTimer);

	bIsReleasingOwner = false;
}

void UTsnStanceObstacleComponent::DrawDebugObstacleState() const
{
#if ENABLE_DRAW_DEBUG
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWorld* World = OwnerActor->GetWorld();
	if (!World) return;

	float EffectiveObstacleRadius = 0.f;
	float EffectiveRepulsionRadius = 0.f;
	GetSanitizedRadii(EffectiveObstacleRadius, EffectiveRepulsionRadius);
	const float EffectiveNavModifierRadius =
		GetEffectiveNavModifierRadius(EffectiveObstacleRadius);
	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const bool bNavModifierActive =
		bUseNavModifier && NavModifierComp && NavModifierComp->IsNavigationRelevant();

	const FString NavAreaLabel = !bUseNavModifier
		? TEXT("Disabled")
		: (NavModifierMode == ETsnNavModifierMode::Impassable
			? TEXT("NavArea_Null")
			: TEXT("TsnNavArea_StanceUnit"));
	const FString MobilityLabel =
		CurrentMobilityStance == ETsnMobilityStance::Stance ? TEXT("Stance") : TEXT("Moving");
	const FString DebugLabel = FString::Printf(
		TEXT("%s | Nav=%s | Modifier=%s\nObstacle=%.0f Nav=%.0f Repulsion=%.0f"),
		*MobilityLabel,
		*NavAreaLabel,
		bNavModifierActive ? TEXT("On") : TEXT("Off"),
		EffectiveObstacleRadius,
		EffectiveNavModifierRadius,
		EffectiveRepulsionRadius);

	DrawDebugCircle(World, OwnerLocation, EffectiveObstacleRadius, 32,
		FColor::Red, false, -1.f, 0, 1.5f,
		FVector::RightVector, FVector::ForwardVector, false);
	DrawDebugCircle(World, OwnerLocation, EffectiveNavModifierRadius, 32,
		bNavModifierActive ? FColor::Cyan : FColor(80, 80, 80), false, -1.f, 0, 1.5f,
		FVector::RightVector, FVector::ForwardVector, false);
	DrawDebugCircle(World, OwnerLocation, EffectiveRepulsionRadius, 32,
		FColor::Yellow, false, -1.f, 0, 1.0f,
		FVector::RightVector, FVector::ForwardVector, false);
	DrawDebugCylinder(World,
		OwnerLocation + FVector(0.f, 0.f, 2.f),
		OwnerLocation + FVector(0.f, 0.f, 80.f),
		EffectiveNavModifierRadius,
		24,
		bNavModifierActive ? FColor::Blue : FColor(60, 60, 60),
		false,
		-1.f,
		0,
		0.5f);
	DrawDebugString(World,
		OwnerLocation + FVector(0.f, 0.f, 110.f),
		DebugLabel,
		nullptr,
		FColor::White,
		0.f,
		true);
#endif
}
