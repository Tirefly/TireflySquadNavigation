// TsnTestTargetDummy.cpp
// 测试用靶标角色 —— 槽位环可视化 + Spline 巡逻

#include "TsnTestTargetDummy.h"
#include "TsnTestLog.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

ATsnTestTargetDummy::ATsnTestTargetDummy()
{
	PrimaryActorTick.bCanEverTick = true;

	EngagementSlotComp = CreateDefaultSubobject<UTsnEngagementSlotComponent>(
		TEXT("EngagementSlotComp"));

	// 胶囊体可见，白色
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetHiddenInGame(false);
		Capsule->SetVisibility(true);
		Capsule->ShapeColor = FColor::White;
	}
}

void ATsnTestTargetDummy::BeginPlay()
{
	Super::BeginPlay();
	CachedPatrolSplineComponent.Reset();
	bHasLoggedInvalidPatrolSetup = false;

	UE_LOG(LogTsnTest, Log, TEXT("TargetDummy [%s] spawned at %s, Patrol=%s"),
		*GetName(), *GetActorLocation().ToString(),
		bPatrolAlongSpline ? TEXT("true") : TEXT("false"));
}

USplineComponent* ATsnTestTargetDummy::ResolvePatrolSplineComponent()
{
	if (CachedPatrolSplineComponent.IsValid())
	{
		return CachedPatrolSplineComponent.Get();
	}

	if (!SplineActor)
	{
		if (!bHasLoggedInvalidPatrolSetup)
		{
			UE_LOG(LogTsnTest, Warning,
				TEXT("TargetDummy [%s] patrol is enabled but SplineActor is null."),
				*GetName());
			bHasLoggedInvalidPatrolSetup = true;
		}
		return nullptr;
	}

	CachedPatrolSplineComponent = SplineActor->FindComponentByClass<USplineComponent>();
	if (!CachedPatrolSplineComponent.IsValid() && !bHasLoggedInvalidPatrolSetup)
	{
		UE_LOG(LogTsnTest, Warning,
			TEXT("TargetDummy [%s] patrol actor [%s] has no SplineComponent. Use an Actor that owns a SplineComponent, not a SplineMeshActor."),
			*GetName(), *SplineActor->GetName());
		bHasLoggedInvalidPatrolSetup = true;
	}

	return CachedPatrolSplineComponent.Get();
}

void ATsnTestTargetDummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Spline 巡逻
	if (bPatrolAlongSpline)
	{
		USplineComponent* SplineComp = ResolvePatrolSplineComponent();
		if (SplineComp)
		{
			const float SplineLen = SplineComp->GetSplineLength();
			SplineDistance += PatrolSpeed * DeltaSeconds * PatrolDirection;

			// 到达末端则折返
			if (SplineDistance >= SplineLen)
			{
				SplineDistance = SplineLen;
				PatrolDirection = -1.f;
			}
			else if (SplineDistance <= 0.f)
			{
				SplineDistance = 0.f;
				PatrolDirection = 1.f;
			}

			const FVector NewLoc = SplineComp->GetLocationAtDistanceAlongSpline(
				SplineDistance, ESplineCoordinateSpace::World);
			SetActorLocation(NewLoc);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugSlotRings)
	{
		DrawDebugSlotVisualization();
	}
#endif
}

#if ENABLE_DRAW_DEBUG
void ATsnTestTargetDummy::DrawDebugSlotVisualization() const
{
	if (!EngagementSlotComp) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Center = GetActorLocation();
	const int32 OccupiedCount = EngagementSlotComp->GetOccupiedSlotCount();
	const int32 MaxSlots = EngagementSlotComp->MaxSlots;

	// 头顶文字：槽位占用
	DrawDebugString(World,
		Center + FVector(0.f, 0.f, 120.f),
		FString::Printf(TEXT("Slots: %d/%d"), OccupiedCount, MaxSlots),
		nullptr, FColor::Yellow, 0.f, true);

	// 绘制常见环半径参考圈（近战/长矛/远程）
	static const float RingRadii[] = { 130.f, 280.f, 580.f };
	static const FColor RingColors[] = { FColor::Red, FColor::Green, FColor::Blue };
	static const TCHAR* RingNames[] = { TEXT("Melee"), TEXT("Spear"), TEXT("Ranged") };

	for (int32 i = 0; i < 3; ++i)
	{
		DrawDebugCircle(World, Center, RingRadii[i], 48,
			RingColors[i], false, 0.f, 0, 1.5f,
			FVector(1, 0, 0), FVector(0, 1, 0), false);

		// 环标签
		DrawDebugString(World,
			Center + FVector(RingRadii[i] + 20.f, 0.f, 10.f),
			FString::Printf(TEXT("%s R=%.0f"), RingNames[i], RingRadii[i]),
			nullptr, RingColors[i], 0.f, true);
	}
}
#endif
