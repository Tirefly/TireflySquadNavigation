// TsnStanceObstacleComponent.cpp

#include "Components/TsnStanceObstacleComponent.h"
#include "NavModifierComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavAreas/NavArea_Null.h"
#include "NavAreas/TsnNavArea_StanceUnit.h"
#include "Settings/TsnDeveloperSettings.h"
#include "Subsystems/TsnStanceRepulsionSubsystem.h"
#include "TsnLog.h"

UTsnStanceObstacleComponent::UTsnStanceObstacleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTsnStanceObstacleComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheComponents();

	FTsnResolvedStanceObstacleSettings ResolvedSettings;
	GetResolvedSettings(ResolvedSettings);

	if (ResolvedSettings.bUseNavModifier)
	{
		InitNavModifier(ResolvedSettings);
	}
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

void UTsnStanceObstacleComponent::GetResolvedSettings(
	FTsnResolvedStanceObstacleSettings& OutResolvedSettings) const
{
	if (!bOverrideTsnDefaults)
	{
		if (const UTsnDeveloperSettings* Settings = UTsnDeveloperSettings::Get())
		{
			OutResolvedSettings.bUseNavModifier = Settings->GetDefaultUseNavModifier();
			OutResolvedSettings.NavModifierMode = Settings->GetDefaultNavModifierMode();
			OutResolvedSettings.ObstacleRadius = Settings->GetDefaultObstacleRadius();
			OutResolvedSettings.NavModifierExtraRadius = Settings->GetDefaultNavModifierExtraRadius();
			OutResolvedSettings.RepulsionRadius = Settings->GetDefaultRepulsionRadius();
			OutResolvedSettings.RepulsionStrength = Settings->GetDefaultRepulsionStrength();
			OutResolvedSettings.NavModifierDeactivationDelay =
				Settings->GetDefaultNavModifierDeactivationDelay();
			return;
		}
	}

	OutResolvedSettings.bUseNavModifier = bUseNavModifier;
	OutResolvedSettings.NavModifierMode = NavModifierMode;
	OutResolvedSettings.ObstacleRadius = ObstacleRadius;
	OutResolvedSettings.NavModifierExtraRadius = NavModifierExtraRadius;
	OutResolvedSettings.RepulsionRadius = RepulsionRadius;
	OutResolvedSettings.RepulsionStrength = RepulsionStrength;
	OutResolvedSettings.NavModifierDeactivationDelay = NavModifierDeactivationDelay;
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
	const FTsnResolvedStanceObstacleSettings& ResolvedSettings,
	float& OutObstacleRadius,
	float& OutRepulsionRadius) const
{
	OutObstacleRadius = FMath::Max(ResolvedSettings.ObstacleRadius, 10.f);

	const bool bValidOuterBand = ResolvedSettings.RepulsionRadius > OutObstacleRadius;
	ensureMsgf(bValidOuterBand,
		TEXT("TsnStanceObstacleComponent: RepulsionRadius (%.1f) must be > ObstacleRadius (%.1f)."),
		ResolvedSettings.RepulsionRadius, OutObstacleRadius);

	OutRepulsionRadius = bValidOuterBand
		? ResolvedSettings.RepulsionRadius
		: (OutObstacleRadius + 30.f);
}

void UTsnStanceObstacleComponent::GetDebugRadii(
	float& OutObstacleRadius,
	float& OutNavModifierRadius,
	float& OutRepulsionRadius) const
{
	FTsnResolvedStanceObstacleSettings ResolvedSettings;
	GetResolvedSettings(ResolvedSettings);
	GetSanitizedRadii(ResolvedSettings, OutObstacleRadius, OutRepulsionRadius);
	OutNavModifierRadius = GetEffectiveNavModifierRadius(ResolvedSettings, OutObstacleRadius);
}

bool UTsnStanceObstacleComponent::GetCrowdDebugInfo(
	float& OutAgentRadius,
	float& OutAgentHalfHeight,
	float& OutCollisionQueryRange) const
{
	OutAgentRadius = 0.f;
	OutAgentHalfHeight = 0.f;
	OutCollisionQueryRange = 0.f;

	if (!CrowdFollowingComp)
	{
		const_cast<UTsnStanceObstacleComponent*>(this)->CacheComponents();
	}

	if (!CrowdFollowingComp)
	{
		return false;
	}

	CrowdFollowingComp->GetCrowdAgentCollisions(OutAgentRadius, OutAgentHalfHeight);
	OutCollisionQueryRange = CrowdFollowingComp->GetCrowdCollisionQueryRange();
	return true;
}

void UTsnStanceObstacleComponent::GetDebugNavModifierState(
	bool& OutUsesNavModifier,
	bool& OutNavModifierActive,
	ETsnNavModifierMode& OutNavModifierMode) const
{
	FTsnResolvedStanceObstacleSettings ResolvedSettings;
	GetResolvedSettings(ResolvedSettings);

	OutUsesNavModifier = ResolvedSettings.bUseNavModifier;
	OutNavModifierActive = ResolvedSettings.bUseNavModifier
		&& NavModifierComp
		&& NavModifierComp->IsNavigationRelevant();
	OutNavModifierMode = ResolvedSettings.NavModifierMode;
}

float UTsnStanceObstacleComponent::GetEffectiveNavModifierRadius(
	const FTsnResolvedStanceObstacleSettings& ResolvedSettings,
	float SanitizedObstacleRadius) const
{
	return SanitizedObstacleRadius + FMath::Max(ResolvedSettings.NavModifierExtraRadius, 0.f);
}

void UTsnStanceObstacleComponent::InitNavModifier(
	const FTsnResolvedStanceObstacleSettings& ResolvedSettings)
{
	if (NavModifierComp)
	{
		SyncNavModifierConfig(ResolvedSettings);
		NavModifierComp->SetNavigationRelevancy(false);
		return;
	}

	float EffectiveObstacleRadius = 0.f;
	float IgnoredRepulsionRadius = 0.f;
	GetSanitizedRadii(ResolvedSettings, EffectiveObstacleRadius, IgnoredRepulsionRadius);
	const float EffectiveNavModifierRadius = GetEffectiveNavModifierRadius(
		ResolvedSettings,
		EffectiveObstacleRadius);

	// 开关式：BeginPlay 预创建，初始状态关闭
	NavModifierComp = NewObject<UNavModifierComponent>(
		GetOwner(), UNavModifierComponent::StaticClass(),
		TEXT("TsnStanceObstacleNavMod"));

	NavModifierComp->FailsafeExtent =
		FVector(EffectiveNavModifierRadius, EffectiveNavModifierRadius, 100.f);
	ApplyNavAreaClass(ResolvedSettings.NavModifierMode);
	NavModifierComp->SetNavigationRelevancy(false);
	NavModifierComp->RegisterComponent();
}

void UTsnStanceObstacleComponent::SyncNavModifierConfig(
	const FTsnResolvedStanceObstacleSettings& ResolvedSettings)
{
	if (!NavModifierComp)
	{
		return;
	}

	float EffectiveObstacleRadius = 0.f;
	float IgnoredRepulsionRadius = 0.f;
	GetSanitizedRadii(ResolvedSettings, EffectiveObstacleRadius, IgnoredRepulsionRadius);
	const float EffectiveNavModifierRadius = GetEffectiveNavModifierRadius(
		ResolvedSettings,
		EffectiveObstacleRadius);

	NavModifierComp->FailsafeExtent =
		FVector(EffectiveNavModifierRadius, EffectiveNavModifierRadius, 100.f);
	ApplyNavAreaClass(ResolvedSettings.NavModifierMode);
}

void UTsnStanceObstacleComponent::ApplyNavAreaClass(
	ETsnNavModifierMode InNavModifierMode)
{
	if (!NavModifierComp) return;

	switch (InNavModifierMode)
	{
	case ETsnNavModifierMode::Impassable:
		NavModifierComp->SetAreaClass(UNavArea_Null::StaticClass());
		break;
	case ETsnNavModifierMode::HighCost:
		NavModifierComp->SetAreaClass(UTsnNavArea_StanceUnit::StaticClass());
		break;
	}
}

void UTsnStanceObstacleComponent::ActivateNavModifier(
	const FTsnResolvedStanceObstacleSettings& ResolvedSettings)
{
	InitNavModifier(ResolvedSettings);
	if (!NavModifierComp) return;

	GetWorld()->GetTimerManager().ClearTimer(NavModifierDeactivationTimer);
	SyncNavModifierConfig(ResolvedSettings);
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

	FTsnResolvedStanceObstacleSettings ResolvedSettings;
	GetResolvedSettings(ResolvedSettings);

	// 补缓存（BeginPlay 时 Controller 可能还未 Possess）
	if (!CrowdFollowingComp) CacheComponents();

	if (CrowdFollowingComp)
		CrowdFollowingComp->SetCrowdSimulationState(ECrowdSimulationState::ObstacleOnly);

	if (ResolvedSettings.bUseNavModifier)
	{
		ActivateNavModifier(ResolvedSettings);
	}
	else
	{
		DeactivateNavModifier();
	}

	float EffectiveObstacleRadius = 0.f;
	float EffectiveRepulsionRadius = 0.f;
	GetSanitizedRadii(ResolvedSettings, EffectiveObstacleRadius, EffectiveRepulsionRadius);

	if (UTsnStanceRepulsionSubsystem* RepSys =
			GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
	{
		RepSys->RegisterStanceUnit(
			GetOwner(),
			EffectiveRepulsionRadius,
			ResolvedSettings.RepulsionStrength,
			EffectiveObstacleRadius);
	}
}

void UTsnStanceObstacleComponent::ExitStanceMode()
{
	if (CurrentMobilityStance == ETsnMobilityStance::Moving) return;
	CurrentMobilityStance = ETsnMobilityStance::Moving;

	FTsnResolvedStanceObstacleSettings ResolvedSettings;
	GetResolvedSettings(ResolvedSettings);

	if (CrowdFollowingComp)
		CrowdFollowingComp->SetCrowdSimulationState(ECrowdSimulationState::Enabled);

	// 延迟关闭：避免其他单位路径突然跳变；对象池回收时立即关闭
	if (ResolvedSettings.bUseNavModifier && NavModifierComp)
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
				ResolvedSettings.NavModifierDeactivationDelay,
				false);
		}
	}
	else
	{
		DeactivateNavModifier();
	}

	if (UTsnStanceRepulsionSubsystem* RepSys =
			GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
	{
		RepSys->UnregisterStanceUnit(GetOwner());
	}
}

void UTsnStanceObstacleComponent::UpdateStanceUnitParams()
{
	FTsnResolvedStanceObstacleSettings ResolvedSettings;
	GetResolvedSettings(ResolvedSettings);

	// CDO、模板对象或尚未拥有有效 World 的阶段，只允许更新本地参数，
	// 不能提前创建/注册运行时 NavModifier 或访问定时器与子系统。
	if (IsTemplate() || !IsValid(GetOwner()) || GetWorld() == nullptr)
	{
		return;
	}

	float EffectiveObstacleRadius = 0.f;
	float EffectiveRepulsionRadius = 0.f;
	GetSanitizedRadii(ResolvedSettings, EffectiveObstacleRadius, EffectiveRepulsionRadius);
	const float EffectiveNavModifierRadius = GetEffectiveNavModifierRadius(
		ResolvedSettings,
		EffectiveObstacleRadius);

	if (ResolvedSettings.bUseNavModifier)
	{
		InitNavModifier(ResolvedSettings);
		SyncNavModifierConfig(ResolvedSettings);

		if (CurrentMobilityStance == ETsnMobilityStance::Stance)
		{
			NavModifierComp->SetNavigationRelevancy(true);
		}

		NavModifierComp->FailsafeExtent =
			FVector(EffectiveNavModifierRadius, EffectiveNavModifierRadius, 100.f);
		NavModifierComp->UpdateNavigationBounds();

		if (UNavigationSystemV1* NavSys =
				FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			NavSys->UpdateComponentInNavOctree(*NavModifierComp);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(NavModifierDeactivationTimer);
		DeactivateNavModifier();
	}

	if (CurrentMobilityStance == ETsnMobilityStance::Stance)
	{
		if (UTsnStanceRepulsionSubsystem* RepSys =
				GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
		{
			RepSys->UpdateStanceUnit(
				GetOwner(),
				EffectiveRepulsionRadius,
				ResolvedSettings.RepulsionStrength,
				EffectiveObstacleRadius);
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
