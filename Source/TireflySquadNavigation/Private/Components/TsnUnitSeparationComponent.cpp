// TsnUnitSeparationComponent.cpp

#include "Components/TsnUnitSeparationComponent.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Subsystems/TsnStanceRepulsionSubsystem.h"

UTsnUnitSeparationComponent::UTsnUnitSeparationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.033f; // ~30Hz，节省性能
}

void UTsnUnitSeparationComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		CachedRepulsionSubsystem = World->GetSubsystem<UTsnStanceRepulsionSubsystem>();
	}
}

void UTsnUnitSeparationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CachedRepulsionSubsystem = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UTsnUnitSeparationComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (!MyPawn || !CachedRepulsionSubsystem) return;

	// 只对移动中的单位生效
	UTsnStanceObstacleComponent* ObsComp =
		MyPawn->FindComponentByClass<UTsnStanceObstacleComponent>();
	if (ObsComp && ObsComp->GetMobilityStance() != ETsnMobilityStance::Moving) return;

	FVector MyLoc = MyPawn->GetActorLocation();
	FVector SeparationDir = FVector::ZeroVector;
	TArray<FVector> InwardToStanceDirs;

	// Moving↔Moving：遍历子系统注册列表做直接距离查询，避免物理 Overlap 开销
	// MovingUnits 包含所有已注册的战术单位（含 Stance），需手动过滤当前为 Stance 的项
	for (const TWeakObjectPtr<AActor>& WeakOther : CachedRepulsionSubsystem->GetMovingUnits())
	{
		AActor* Other = WeakOther.Get();
		if (!Other || Other == MyPawn) continue;

		// 跳过当前处于 Stance 的单位（Stance↔Moving 由 RepulsionSubsystem 独占负责）
		UTsnStanceObstacleComponent* OtherObsComp =
			Other->FindComponentByClass<UTsnStanceObstacleComponent>();
		if (OtherObsComp && OtherObsComp->GetMobilityStance() == ETsnMobilityStance::Stance)
			continue;

		FVector Diff = MyLoc - Other->GetActorLocation();
		float Dist2D = Diff.Size2D();
		if (Dist2D <= KINDA_SMALL_NUMBER || Dist2D >= SeparationRadius) continue;

		float Factor = 1.f - (Dist2D / SeparationRadius);
		SeparationDir += Diff.GetSafeNormal2D() * Factor;
	}

	// Stance 约束方向：来自 StanceUnits 列表，仅记录方向供后续裁剪
	for (const FTsnStanceObstacle& Obs : CachedRepulsionSubsystem->GetStanceUnits())
	{
		if (!Obs.Unit.IsValid()) continue;
		FVector StanceLoc = Obs.Unit->GetActorLocation();
		float Dist2D = (MyLoc - StanceLoc).Size2D();
		if (Dist2D >= SeparationRadius || Dist2D <= KINDA_SMALL_NUMBER) continue;
		// 朝内方向 = 从自身指向站桩单位（进入内侧的方向）
		InwardToStanceDirs.Add((StanceLoc - MyLoc).GetSafeNormal2D());
	}

	FVector AdjustedSeparation = SeparationDir.GetClampedToMaxSize(1.f);
	if (InwardToStanceDirs.Num() > 0)
	{
		// 累加所有站桩单位的 InwardToStance 方向，归一化后做单次投影裁剪
		// 这样不会因多个站桩单位从不同方向约束而过度裁剪 Separation
		FVector AccumulatedInward = FVector::ZeroVector;
		for (const FVector& InwardToStance : InwardToStanceDirs)
		{
			AccumulatedInward += InwardToStance;
		}
		AccumulatedInward = AccumulatedInward.GetSafeNormal2D();
		if (!AccumulatedInward.IsNearlyZero())
		{
			float InwardDot = FVector::DotProduct(AdjustedSeparation, AccumulatedInward);
			if (InwardDot > 0.f)
			{
				// 移除指向站桩单位内侧的分量，只允许切向/外向的软协调输入保留
				AdjustedSeparation -= AccumulatedInward * InwardDot;
			}
		}
	}

	if (!AdjustedSeparation.IsNearlyZero())
	{
		// 通过 AddMovementInput 注入；进入站桩 keep-out 邻域时，
		// 该输入已先被裁剪，不会把单位继续推向站桩单位。
		//
		// 【DeltaTime 不参与此注入】
		// AddMovementInput 仅将 SeparationStrength 写入 PendingMovementInputVector，
		// 不乘以本帧 DeltaTime。最终速度积分 (Velocity += Accel * DeltaSeconds) 由 CMC
		// 在其自己的 TickComponent 中完成，与本组件的 30Hz tick 间隔无关。
		// 因此 30Hz → 60Hz 帧率变化不会导致分离力脉冲放大或缩小。
		MyPawn->AddMovementInput(AdjustedSeparation.GetSafeNormal2D(), SeparationStrength);
	}
}

void UTsnUnitSeparationComponent::OnOwnerReleased()
{
	SetComponentTickEnabled(false);
	CachedRepulsionSubsystem = nullptr;
}
