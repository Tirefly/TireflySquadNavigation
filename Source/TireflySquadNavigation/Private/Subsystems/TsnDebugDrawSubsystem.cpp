// TsnDebugDrawSubsystem.cpp

#include "Subsystems/TsnDebugDrawSubsystem.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "Components/TsnUnitSeparationComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/IConsoleManager.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "Subsystems/TsnEngagementSlotSubsystem.h"
#include "Subsystems/TsnStanceRepulsionSubsystem.h"

namespace
{
	enum class ETsnUnitOverlayPreset : uint8
	{
		Overview = 0,
		Focus = 1,
		Tsn = 2,
		Crowd = 3,
	};

	enum class ETsnUnitOverlayFilterMode : uint8
	{
		All = 0,
		Nearest = 1,
		Name = 2,
	};

	enum ETsnUnitOverlayLayerFlags : uint32
	{
		Overlay_Capsule = 1 << 0,
		Overlay_Crowd = 1 << 1,
		Overlay_Separation = 1 << 2,
		Overlay_Engagement = 1 << 3,
		Overlay_Obstacle = 1 << 4,
		Overlay_NavModifier = 1 << 5,
		Overlay_Repulsion = 1 << 6,
		Overlay_Slots = 1 << 7,
		Overlay_Text = 1 << 8,
		Overlay_Vectors = 1 << 9,
	};

	static int32 GDrawDebugTsnUnitOverlay = 0;
	static FAutoConsoleVariableRef CVarDrawDebugTsnUnitOverlay(
		TEXT("tsn.debug.DrawUnitOverlay"),
		GDrawDebugTsnUnitOverlay,
		TEXT("1 = Draw centralized TSN unit overlay for capsule, engagement, TSN, Crowd and slot radii (Development/Debug only)"),
		ECVF_Default);

	static int32 GTsnUnitOverlayPreset = static_cast<int32>(ETsnUnitOverlayPreset::Overview);
	static FAutoConsoleVariableRef CVarTsnUnitOverlayPreset(
		TEXT("tsn.debug.UnitOverlayPreset"),
		GTsnUnitOverlayPreset,
		TEXT("Centralized TSN unit overlay preset. 0=Overview, 1=Focus, 2=TSN, 3=Crowd"),
		ECVF_Default);

	static int32 GTsnUnitOverlayMask = 0;
	static FAutoConsoleVariableRef CVarTsnUnitOverlayMask(
		TEXT("tsn.debug.UnitOverlayMask"),
		GTsnUnitOverlayMask,
		TEXT("Optional override mask for centralized TSN unit overlay. 0 = use preset mask."),
		ECVF_Default);

	static int32 GTsnUnitOverlayText = 1;
	static FAutoConsoleVariableRef CVarTsnUnitOverlayText(
		TEXT("tsn.debug.UnitOverlayText"),
		GTsnUnitOverlayText,
		TEXT("1 = draw centralized TSN unit overlay text labels when the preset or mask includes text."),
		ECVF_Default);

	static int32 GTsnUnitOverlayTargetPair = 1;
	static FAutoConsoleVariableRef CVarTsnUnitOverlayTargetPair(
		TEXT("tsn.debug.UnitOverlayTargetPair"),
		GTsnUnitOverlayTargetPair,
		TEXT("1 = when focusing a unit, also draw its current target-side slot overlay if available."),
		ECVF_Default);

	static int32 GTsnUnitOverlayFilterMode = static_cast<int32>(ETsnUnitOverlayFilterMode::All);
	static FAutoConsoleVariableRef CVarTsnUnitOverlayFilterMode(
		TEXT("tsn.debug.UnitOverlayFilterMode"),
		GTsnUnitOverlayFilterMode,
		TEXT("Centralized TSN unit overlay filter mode. 0=All, 1=Nearest, 2=Name"),
		ECVF_Default);

	static float GTsnUnitOverlayLaneSpacing = 24.f;
	static FAutoConsoleVariableRef CVarTsnUnitOverlayLaneSpacing(
		TEXT("tsn.debug.UnitOverlayLaneSpacing"),
		GTsnUnitOverlayLaneSpacing,
		TEXT("Vertical spacing in cm between centralized TSN unit overlay lanes."),
		ECVF_Default);

	static float GTsnUnitOverlaySlotHeightOffset = 120.f;
	static FAutoConsoleVariableRef CVarTsnUnitOverlaySlotHeightOffset(
		TEXT("tsn.debug.UnitOverlaySlotHeightOffset"),
		GTsnUnitOverlaySlotHeightOffset,
		TEXT("Additional Z offset in cm applied to target-slot circles, occupancy rings and arrows."),
		ECVF_Default);

	static int32 GTsnUnitOverlayMaxUnits = 24;
	static FAutoConsoleVariableRef CVarTsnUnitOverlayMaxUnits(
		TEXT("tsn.debug.UnitOverlayMaxUnits"),
		GTsnUnitOverlayMaxUnits,
		TEXT("Maximum number of units drawn in centralized TSN unit overlay when using the All filter."),
		ECVF_Default);

	static FString GTsnUnitOverlayFocusName;

	static void SetOverlayFocusName(const TArray<FString>& Args)
	{
		GTsnUnitOverlayFocusName = Args.Num() > 0
			? FString::Join(Args, TEXT(" "))
			: FString();
		GTsnUnitOverlayFilterMode = static_cast<int32>(ETsnUnitOverlayFilterMode::Name);
	}

	static FAutoConsoleCommand CmdTsnUnitOverlayFocusName(
		TEXT("tsn.debug.UnitOverlayFocusName"),
		TEXT("Usage: tsn.debug.UnitOverlayFocusName <ActorNameSubstring>"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SetOverlayFocusName));

	static FAutoConsoleCommand CmdTsnUnitOverlayFocusNearest(
		TEXT("tsn.debug.UnitOverlayFocusNearest"),
		TEXT("Focus centralized TSN unit overlay on the nearest tactical unit to the local camera."),
		FConsoleCommandDelegate::CreateStatic([]()
		{
			GTsnUnitOverlayFilterMode = static_cast<int32>(ETsnUnitOverlayFilterMode::Nearest);
		}));

	static FAutoConsoleCommand CmdTsnUnitOverlayFocusAll(
		TEXT("tsn.debug.UnitOverlayFocusAll"),
		TEXT("Draw centralized TSN unit overlay for all nearby tactical units."),
		FConsoleCommandDelegate::CreateStatic([]()
		{
			GTsnUnitOverlayFilterMode = static_cast<int32>(ETsnUnitOverlayFilterMode::All);
			GTsnUnitOverlayFocusName.Reset();
		}));

	static FAutoConsoleCommand CmdTsnUnitOverlayClearFocusName(
		TEXT("tsn.debug.UnitOverlayClearFocusName"),
		TEXT("Clear the name filter used by the centralized TSN unit overlay."),
		FConsoleCommandDelegate::CreateStatic([]()
		{
			GTsnUnitOverlayFocusName.Reset();
			if (GTsnUnitOverlayFilterMode == static_cast<int32>(ETsnUnitOverlayFilterMode::Name))
			{
				GTsnUnitOverlayFilterMode = static_cast<int32>(ETsnUnitOverlayFilterMode::All);
			}
		}));

	int32 GetLaneIndex(uint32 LayerFlag)
	{
		switch (LayerFlag)
		{
		case Overlay_Capsule:
			return 0;
		case Overlay_Crowd:
			return 1;
		case Overlay_Separation:
			return 2;
		case Overlay_Engagement:
			return 3;
		case Overlay_Obstacle:
			return 4;
		case Overlay_NavModifier:
			return 5;
		case Overlay_Repulsion:
			return 6;
		case Overlay_Slots:
			return 7;
		case Overlay_Text:
		default:
			return 8;
		}
	}

	FColor GetOverlayColor(uint32 LayerFlag)
	{
		switch (LayerFlag)
		{
		case Overlay_Capsule:
			return FColor::White;
		case Overlay_Crowd:
			return FColor(255, 140, 0);
		case Overlay_Separation:
			return FColor::Cyan;
		case Overlay_Engagement:
			return FColor::Yellow;
		case Overlay_Obstacle:
			return FColor::Red;
		case Overlay_NavModifier:
			return FColor::Green;
		case Overlay_Repulsion:
			return FColor::Magenta;
		case Overlay_Slots:
			return FColor::Blue;
		case Overlay_Text:
		case Overlay_Vectors:
		default:
			return FColor::White;
		}
	}

	const TCHAR* GetNavModifierModeLabel(ETsnNavModifierMode Mode)
	{
		switch (Mode)
		{
		case ETsnNavModifierMode::HighCost:
			return TEXT("HighCost");
		case ETsnNavModifierMode::Impassable:
		default:
			return TEXT("Null");
		}
	}

	uint32 GetPresetMask(ETsnUnitOverlayPreset Preset)
	{
		switch (Preset)
		{
		case ETsnUnitOverlayPreset::Focus:
			return Overlay_Capsule | Overlay_Crowd | Overlay_Separation | Overlay_Engagement
				| Overlay_Obstacle | Overlay_NavModifier | Overlay_Repulsion | Overlay_Slots
				| Overlay_Text | Overlay_Vectors;
		case ETsnUnitOverlayPreset::Tsn:
			return Overlay_Engagement | Overlay_Obstacle | Overlay_NavModifier
				| Overlay_Repulsion | Overlay_Slots | Overlay_Text | Overlay_Vectors;
		case ETsnUnitOverlayPreset::Crowd:
			return Overlay_Capsule | Overlay_Crowd | Overlay_Separation
				| Overlay_Text | Overlay_Vectors;
		case ETsnUnitOverlayPreset::Overview:
		default:
			return Overlay_Capsule | Overlay_Engagement | Overlay_Repulsion;
		}
	}

	FVector GetViewOrigin(const UWorld* World)
	{
		if (!World)
		{
			return FVector::ZeroVector;
		}

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (const APlayerController* PlayerController = It->Get())
			{
				if (const APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
				{
					return CameraManager->GetCameraLocation();
				}

				if (const APawn* Pawn = PlayerController->GetPawn())
				{
					return Pawn->GetActorLocation();
				}
			}
		}

		return FVector::ZeroVector;
	}

	bool MatchesNameFilter(const AActor* Actor)
	{
		if (!Actor || GTsnUnitOverlayFocusName.IsEmpty())
		{
			return false;
		}

		return Actor->GetName().Contains(GTsnUnitOverlayFocusName, ESearchCase::IgnoreCase);
	}

	float GetLayerHeightOffset(uint32 LayerFlag, float LaneSpacing)
	{
		const float BaseLaneOffset = LaneSpacing * GetLaneIndex(LayerFlag);
		if (LayerFlag == Overlay_Slots)
		{
			return BaseLaneOffset + FMath::Max(0.f, GTsnUnitOverlaySlotHeightOffset);
		}

		return BaseLaneOffset;
	}

	void DrawOverlayCircle(
		UWorld* World,
		const FVector& Center,
		float Radius,
		uint32 LayerFlag,
		float LaneSpacing,
		float Thickness = 1.25f)
	{
		if (!World || Radius <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		const FVector LayerCenter = Center + FVector(0.f, 0.f, GetLayerHeightOffset(LayerFlag, LaneSpacing));
		DrawDebugCircle(
			World,
			LayerCenter,
			Radius,
			48,
			GetOverlayColor(LayerFlag),
			false,
			0.f,
			0,
			Thickness,
			FVector::RightVector,
			FVector::ForwardVector,
			false);
	}

	void DrawOverlayArrow(
		UWorld* World,
		const FVector& Base,
		const FVector& Direction,
		const FColor& Color,
		float Scale,
		float LaneOffset)
	{
		if (!World || Direction.IsNearlyZero())
		{
			return;
		}

		const FVector Start = Base + FVector(0.f, 0.f, LaneOffset);
		DrawDebugDirectionalArrow(
			World,
			Start,
			Start + Direction.GetClampedToMaxSize(Scale),
			20.f,
			Color,
			false,
			0.f,
			0,
			2.f);
	}

	void DrawAssignedSlotOccupancy(
		UWorld* World,
		const FVector& UnitLocation,
		const FVector& SlotWorldPosition,
		float OccupantRadius,
		const FColor& Color,
		float LaneSpacing)
	{
		if (!World)
		{
			return;
		}

		const float SlotLaneOffset = GetLayerHeightOffset(Overlay_Slots, LaneSpacing);
		const FVector SlotLanePosition = SlotWorldPosition + FVector(0.f, 0.f, SlotLaneOffset);
		const FVector UnitLanePosition = UnitLocation + FVector(0.f, 0.f, SlotLaneOffset);

		DrawDebugSphere(
			World,
			SlotLanePosition,
			14.f,
			8,
			Color,
			false,
			0.f,
			0,
			1.5f);

		DrawDebugCircle(
			World,
			SlotLanePosition,
			FMath::Max(OccupantRadius, 1.f),
			32,
			Color,
			false,
			0.f,
			0,
			1.75f,
			FVector::RightVector,
			FVector::ForwardVector,
			false);

		DrawDebugDirectionalArrow(
			World,
			UnitLanePosition,
			SlotLanePosition,
			18.f,
			Color,
			false,
			0.f,
			0,
			1.5f);
	}

	AActor* ResolveOverlayTarget(UWorld* World, AActor* SourceActor)
	{
		if (!World || !SourceActor)
		{
			return nullptr;
		}

		if (UTsnEngagementSlotSubsystem* SlotSubsystem = World->GetSubsystem<UTsnEngagementSlotSubsystem>())
		{
			if (UTsnEngagementSlotComponent* OccupiedSlotComponent = SlotSubsystem->FindOccupiedSlotComponent(SourceActor))
			{
				return OccupiedSlotComponent->GetOwner();
			}
		}

		if (const APawn* Pawn = Cast<APawn>(SourceActor))
		{
			if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
			{
				if (const UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
				{
					return Cast<AActor>(BlackboardComponent->GetValueAsObject(TEXT("TargetActor")));
				}
			}
		}

		return nullptr;
	}

	void DrawTargetSlotOverlay(
		UWorld* World,
		AActor* TargetActor,
		AActor* Requester,
		float LaneSpacing)
	{
		if (!World || !TargetActor)
		{
			return;
		}

		UTsnEngagementSlotComponent* SlotComponent = TargetActor->FindComponentByClass<UTsnEngagementSlotComponent>();
		if (!SlotComponent)
		{
			return;
		}

		const FVector TargetLocation = TargetActor->GetActorLocation();
		const FVector RequesterLocation = Requester ? Requester->GetActorLocation() : FVector::ZeroVector;
		const UCapsuleComponent* RequesterCapsule = Requester
			? Requester->FindComponentByClass<UCapsuleComponent>()
			: nullptr;
		const float RequesterRadius = RequesterCapsule
			? RequesterCapsule->GetScaledCapsuleRadius()
			: 0.f;
		TArray<FTsnEngagementSlotInfo> AssignedSlots;
		SlotComponent->GetAssignedSlots(AssignedSlots);

		TSet<int32> DrawnRadii;
		for (const FTsnEngagementSlotInfo& SlotInfo : AssignedSlots)
		{
			const FVector SlotWorldPosition = SlotComponent->GetSlotWorldPosition(SlotInfo);
			const FColor SlotColor = SlotInfo.Occupant == Requester
				? FColor::Yellow
				: GetOverlayColor(Overlay_Slots);

			if (SlotInfo.Occupant == Requester && Requester)
			{
				DrawAssignedSlotOccupancy(
					World,
					RequesterLocation,
					SlotWorldPosition,
					RequesterRadius,
					SlotColor,
					LaneSpacing);
			}
			else
			{
				DrawDebugSphere(
					World,
					SlotWorldPosition + FVector(0.f, 0.f, GetLayerHeightOffset(Overlay_Slots, LaneSpacing)),
					14.f,
					8,
					SlotColor,
					false,
					0.f,
					0,
					1.5f);
			}
			DrawDebugLine(
				World,
				TargetLocation + FVector(0.f, 0.f, GetLayerHeightOffset(Overlay_Slots, LaneSpacing)),
				SlotWorldPosition + FVector(0.f, 0.f, GetLayerHeightOffset(Overlay_Slots, LaneSpacing)),
				SlotColor,
				false,
				0.f,
				0,
				0.85f);

			const int32 RadiusKey = FMath::RoundToInt(SlotInfo.Radius);
			if (!DrawnRadii.Contains(RadiusKey))
			{
				DrawnRadii.Add(RadiusKey);
				DrawOverlayCircle(World, TargetLocation, SlotInfo.Radius, Overlay_Slots, LaneSpacing, 1.25f);
			}
		}

		if (GTsnUnitOverlayText != 0)
		{
			DrawDebugString(
				World,
				TargetLocation + FVector(0.f, 0.f, LaneSpacing * GetLaneIndex(Overlay_Text) + 36.f),
				FString::Printf(TEXT("PAIR TARGET %s\nSlots=%d/%d"), *TargetActor->GetName(), SlotComponent->GetOccupiedSlotCount(), SlotComponent->MaxSlots),
				nullptr,
				FColor::Blue,
				0.f,
				true);
		}
	}

	void DrawUnitOverlay(
		UWorld* World,
		AActor* UnitActor,
		uint32 LayerMask,
		float LaneSpacing,
		bool bDrawText,
		bool bAllowPairTarget)
	{
		if (!World || !UnitActor)
		{
			return;
		}

		const FVector UnitLocation = UnitActor->GetActorLocation();
		const UCapsuleComponent* CapsuleComponent = UnitActor->FindComponentByClass<UCapsuleComponent>();
		const float CapsuleRadius = CapsuleComponent ? CapsuleComponent->GetScaledCapsuleRadius() : 0.f;
		UTsnStanceObstacleComponent* StanceObstacleComponent = UnitActor->FindComponentByClass<UTsnStanceObstacleComponent>();
		UTsnUnitSeparationComponent* SeparationComponent = UnitActor->FindComponentByClass<UTsnUnitSeparationComponent>();
		UTsnTacticalMovementComponent* TacticalMovementComponent = UnitActor->FindComponentByClass<UTsnTacticalMovementComponent>();

		float EngagementRange = 0.f;
		if (UnitActor->Implements<UTsnTacticalUnit>())
		{
			EngagementRange = ITsnTacticalUnit::Execute_GetEngagementRange(UnitActor);
		}

		float ObstacleRadius = 0.f;
		float NavModifierRadius = 0.f;
		float RepulsionRadius = 0.f;
		float CrowdAgentRadius = 0.f;
		float CrowdAgentHalfHeight = 0.f;
		float CrowdCollisionQueryRange = 0.f;
		if (StanceObstacleComponent)
		{
			StanceObstacleComponent->GetDebugRadii(ObstacleRadius, NavModifierRadius, RepulsionRadius);
			StanceObstacleComponent->GetCrowdDebugInfo(
				CrowdAgentRadius,
				CrowdAgentHalfHeight,
				CrowdCollisionQueryRange);
		}

		if ((LayerMask & Overlay_Capsule) != 0)
		{
			DrawOverlayCircle(World, UnitLocation, CapsuleRadius, Overlay_Capsule, LaneSpacing, 1.5f);
		}

		if ((LayerMask & Overlay_Crowd) != 0)
		{
			DrawOverlayCircle(World, UnitLocation, CrowdAgentRadius, Overlay_Crowd, LaneSpacing, 1.5f);
			DrawOverlayCircle(World, UnitLocation, CrowdCollisionQueryRange, Overlay_Crowd, LaneSpacing, 0.75f);
		}

		if ((LayerMask & Overlay_Separation) != 0 && SeparationComponent)
		{
			DrawOverlayCircle(World, UnitLocation, SeparationComponent->SeparationRadius, Overlay_Separation, LaneSpacing, 1.25f);
		}

		if ((LayerMask & Overlay_Engagement) != 0)
		{
			DrawOverlayCircle(World, UnitLocation, EngagementRange, Overlay_Engagement, LaneSpacing, 1.25f);
		}

		if ((LayerMask & Overlay_Obstacle) != 0)
		{
			DrawOverlayCircle(World, UnitLocation, ObstacleRadius, Overlay_Obstacle, LaneSpacing, 1.5f);
		}

		if ((LayerMask & Overlay_NavModifier) != 0)
		{
			DrawOverlayCircle(World, UnitLocation, NavModifierRadius, Overlay_NavModifier, LaneSpacing, 1.5f);
		}

		if ((LayerMask & Overlay_Repulsion) != 0)
		{
			DrawOverlayCircle(World, UnitLocation, RepulsionRadius, Overlay_Repulsion, LaneSpacing, 1.25f);
		}

		AActor* PairTarget = nullptr;
		UTsnEngagementSlotSubsystem* SlotSubsystem = World->GetSubsystem<UTsnEngagementSlotSubsystem>();
		if ((LayerMask & Overlay_Slots) != 0 && SlotSubsystem)
		{
			if (UTsnEngagementSlotComponent* OccupiedSlotComponent = SlotSubsystem->FindOccupiedSlotComponent(UnitActor))
			{
				FTsnEngagementSlotInfo AssignedSlotInfo;
				FVector AssignedSlotWorldPosition;
				if (OccupiedSlotComponent->TryGetAssignedSlotInfo(UnitActor, AssignedSlotInfo, AssignedSlotWorldPosition))
				{
					DrawAssignedSlotOccupancy(
						World,
						UnitLocation,
						AssignedSlotWorldPosition,
						CapsuleRadius,
						FColor::Yellow,
						LaneSpacing);
				}

				PairTarget = OccupiedSlotComponent->GetOwner();
			}
		}

		if ((LayerMask & Overlay_Vectors) != 0)
		{
			const FVector ActualVelocity = FVector(UnitActor->GetVelocity().X, UnitActor->GetVelocity().Y, 0.f);
			DrawOverlayArrow(World, UnitLocation, ActualVelocity, FColor::White, 120.f, LaneSpacing * GetLaneIndex(Overlay_Text) + 15.f);

			if (TacticalMovementComponent)
			{
				DrawOverlayArrow(
					World,
					UnitLocation,
					FVector(
						TacticalMovementComponent->GetLastUpdateRequestedVelocity().X,
						TacticalMovementComponent->GetLastUpdateRequestedVelocity().Y,
						0.f),
					FColor::Green,
					120.f,
					LaneSpacing * GetLaneIndex(Overlay_Text) + 30.f);
				DrawOverlayArrow(
					World,
					UnitLocation,
					TacticalMovementComponent->GetLastConsumedRepulsionVelocity(),
					FColor::Red,
					100.f,
					LaneSpacing * GetLaneIndex(Overlay_Text) + 45.f);
				DrawOverlayArrow(
					World,
					UnitLocation,
					TacticalMovementComponent->GetCachedEscapeVelocity(),
					FColor::Cyan,
					100.f,
					LaneSpacing * GetLaneIndex(Overlay_Text) + 60.f);
			}
		}

		if (bDrawText && (LayerMask & Overlay_Text) != 0)
		{
			const bool bInStance = StanceObstacleComponent
				&& StanceObstacleComponent->GetMobilityStance() == ETsnMobilityStance::Stance;
			bool bUsesNavModifier = false;
			bool bNavModifierActive = false;
			ETsnNavModifierMode NavModifierMode = ETsnNavModifierMode::Impassable;
			if (StanceObstacleComponent)
			{
				StanceObstacleComponent->GetDebugNavModifierState(
					bUsesNavModifier,
					bNavModifierActive,
					NavModifierMode);
			}
			const float ActualSpeed = UnitActor->GetVelocity().Size2D();
			const float RequestedSpeed = TacticalMovementComponent
				? FVector(
					TacticalMovementComponent->GetLastUpdateRequestedVelocity().X,
					TacticalMovementComponent->GetLastUpdateRequestedVelocity().Y,
					0.f).Size()
				: 0.f;
			const bool bEscapeModeActive = TacticalMovementComponent && TacticalMovementComponent->IsEscapeModeActive();
			const TCHAR* NavModifierStateLabel = !bUsesNavModifier
				? TEXT("Disabled")
				: (bNavModifierActive ? TEXT("On") : TEXT("Off"));
			const FString DebugText = FString::Printf(
				TEXT("%s\nState=%s Nav=%s/%s Speed=%.0f Req=%.0f Escape=%s\nCapsule=%.0f Crowd=%.0f/%.0f Query=%.0f Sep=%.0f Eng=%.0f\nObs=%.0f Nav=%.0f Rep=%.0f"),
				*UnitActor->GetName(),
				bInStance ? TEXT("Stance") : TEXT("Moving"),
				NavModifierStateLabel,
				GetNavModifierModeLabel(NavModifierMode),
				ActualSpeed,
				RequestedSpeed,
				bEscapeModeActive ? TEXT("On") : TEXT("Off"),
				CapsuleRadius,
				CrowdAgentRadius,
				CrowdAgentHalfHeight,
				CrowdCollisionQueryRange,
				SeparationComponent ? SeparationComponent->SeparationRadius : 0.f,
				EngagementRange,
				ObstacleRadius,
				NavModifierRadius,
				RepulsionRadius);

			DrawDebugString(
				World,
				UnitLocation + FVector(0.f, 0.f, LaneSpacing * GetLaneIndex(Overlay_Text) + 36.f),
				DebugText,
				nullptr,
				FColor::White,
				0.f,
				true);
		}

		if (bAllowPairTarget)
		{
			if (!PairTarget)
			{
				PairTarget = ResolveOverlayTarget(World, UnitActor);
			}

			if (PairTarget && PairTarget != UnitActor)
			{
				DrawTargetSlotOverlay(World, PairTarget, UnitActor, LaneSpacing);
			}
		}
	}
}

bool UTsnDebugDrawSubsystem::IsTickable() const
{
#if ENABLE_DRAW_DEBUG
	return GDrawDebugTsnUnitOverlay != 0
		&& GetWorld()
		&& GetWorld()->IsGameWorld();
#else
	return false;
#endif
}

void UTsnDebugDrawSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	(void)DeltaTime;

#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld() || GDrawDebugTsnUnitOverlay == 0)
	{
		return;
	}

	UTsnStanceRepulsionSubsystem* RepulsionSubsystem = World->GetSubsystem<UTsnStanceRepulsionSubsystem>();
	if (!RepulsionSubsystem)
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>> UniqueUnits;
	for (const TWeakObjectPtr<AActor>& WeakUnit : RepulsionSubsystem->GetMovingUnits())
	{
		if (AActor* UnitActor = WeakUnit.Get())
		{
			if (UnitActor->Implements<UTsnTacticalUnit>())
			{
				UniqueUnits.Add(UnitActor);
			}
		}
	}
	for (const FTsnStanceObstacle& StanceUnit : RepulsionSubsystem->GetStanceUnits())
	{
		if (AActor* UnitActor = StanceUnit.Unit.Get())
		{
			if (UnitActor->Implements<UTsnTacticalUnit>())
			{
				UniqueUnits.Add(UnitActor);
			}
		}
	}

	TArray<AActor*> CandidateUnits;
	for (const TWeakObjectPtr<AActor>& WeakUnit : UniqueUnits)
	{
		if (AActor* UnitActor = WeakUnit.Get())
		{
			CandidateUnits.Add(UnitActor);
		}
	}

	if (CandidateUnits.Num() == 0)
	{
		return;
	}

	const FVector ViewOrigin = GetViewOrigin(World);
	CandidateUnits.Sort([&ViewOrigin](const AActor& Left, const AActor& Right)
	{
		return FVector::DistSquared(ViewOrigin, Left.GetActorLocation())
			< FVector::DistSquared(ViewOrigin, Right.GetActorLocation());
	});

	const ETsnUnitOverlayPreset Preset = static_cast<ETsnUnitOverlayPreset>(
		FMath::Clamp(GTsnUnitOverlayPreset, 0, static_cast<int32>(ETsnUnitOverlayPreset::Crowd)));
	uint32 LayerMask = GTsnUnitOverlayMask > 0
		? static_cast<uint32>(GTsnUnitOverlayMask)
		: GetPresetMask(Preset);
	const bool bDrawText = GTsnUnitOverlayText != 0;
	const float LaneSpacing = FMath::Max(8.f, GTsnUnitOverlayLaneSpacing);
	ETsnUnitOverlayFilterMode FilterMode = static_cast<ETsnUnitOverlayFilterMode>(
		FMath::Clamp(GTsnUnitOverlayFilterMode, 0, static_cast<int32>(ETsnUnitOverlayFilterMode::Name)));

	if (Preset == ETsnUnitOverlayPreset::Focus && FilterMode == ETsnUnitOverlayFilterMode::All)
	{
		FilterMode = ETsnUnitOverlayFilterMode::Nearest;
	}

	TArray<AActor*> UnitsToDraw;
	switch (FilterMode)
	{
	case ETsnUnitOverlayFilterMode::Nearest:
		if (CandidateUnits.Num() > 0)
		{
			UnitsToDraw.Add(CandidateUnits[0]);
		}
		break;

	case ETsnUnitOverlayFilterMode::Name:
		for (AActor* CandidateUnit : CandidateUnits)
		{
			if (MatchesNameFilter(CandidateUnit))
			{
				UnitsToDraw.Add(CandidateUnit);
			}
		}
		if (Preset == ETsnUnitOverlayPreset::Focus && UnitsToDraw.Num() > 1)
		{
			UnitsToDraw.SetNum(1);
		}
		break;

	case ETsnUnitOverlayFilterMode::All:
	default:
		UnitsToDraw = CandidateUnits;
		if (GTsnUnitOverlayMaxUnits > 0 && UnitsToDraw.Num() > GTsnUnitOverlayMaxUnits)
		{
			UnitsToDraw.SetNum(GTsnUnitOverlayMaxUnits);
		}
		break;
	}

	for (AActor* UnitActor : UnitsToDraw)
	{
		DrawUnitOverlay(
			World,
			UnitActor,
			LayerMask,
			LaneSpacing,
			bDrawText,
			GTsnUnitOverlayTargetPair != 0 && UnitsToDraw.Num() == 1);
	}
#endif
}