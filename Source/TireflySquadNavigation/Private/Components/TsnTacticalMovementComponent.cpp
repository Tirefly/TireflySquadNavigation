// TsnTacticalMovementComponent.cpp

#include "Components/TsnTacticalMovementComponent.h"
#include "Subsystems/TsnStanceRepulsionSubsystem.h"
#include "AIController.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "Navigation/PathFollowingComponent.h"
#include "TsnLog.h"
#include "DrawDebugHelpers.h"

UTsnTacticalMovementComponent::UTsnTacticalMovementComponent()
{
	// 从根源关闭 CMC 内置 RVO Avoidance，防止与 DetourCrowd 双重避障冲突
	bUseRVOAvoidance = false;
}

void UTsnTacticalMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	StationaryAnchorLocation = GetOwner()
		? GetOwner()->GetActorLocation()
		: FVector::ZeroVector;

	// 向排斥力子系统注册为战术移动单位，使 Tick 遍历范围收窄为战术单位
	if (UTsnStanceRepulsionSubsystem* RepSys =
			GetWorld()->GetSubsystem<UTsnStanceRepulsionSubsystem>())
	{
		RepSys->RegisterMovingUnit(GetOwner());
		CachedRepulsionSubsystem = RepSys;
	}
}

void UTsnTacticalMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedRepulsionSubsystem)
	{
		CachedRepulsionSubsystem->UnregisterMovingUnit(GetOwner());
	}

	CachedRepulsionSubsystem = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UTsnTacticalMovementComponent::OnOwnerReleased()
{
	// 等价于 EndPlay 中的注销逻辑，供对象池回收时调用
	if (CachedRepulsionSubsystem)
	{
		CachedRepulsionSubsystem->UnregisterMovingUnit(GetOwner());
	}
	CachedRepulsionSubsystem = nullptr;
	DeferredRepulsionVelocity = FVector::ZeroVector;
	bHasDeferredRepulsion = false;
	DeferredRepulsionWriteFrame = 0;
	LastConsumedRepulsionVelocity = FVector::ZeroVector;
	LastConsumedRepulsionFrame = 0;
	bEscapeModeActive = false;
	bPendingEscapeRepathRequest = false;
	bLastFrameInsideEscapeEnterZone = false;
	bLastFrameInsideEscapeReleaseZone = false;
	EscapeModeTriggerFrameCount = 0;
	EscapeModeClearFrameCount = 0;
	EscapeModeTimeRemaining = 0.f;
	CachedEscapeDirection = FVector::ZeroVector;
	CachedEscapeVelocity = FVector::ZeroVector;
	StationaryAnchorLocation = FVector::ZeroVector;
	StationaryTime = 0.f;
	bHasLoggedStationary = false;
}

bool UTsnTacticalMovementComponent::IsEscapeModeActive() const
{
	return bEscapeModeActive;
}

float UTsnTacticalMovementComponent::GetEscapeModeTimeRemaining() const
{
	return EscapeModeTimeRemaining;
}

FVector UTsnTacticalMovementComponent::GetLastConsumedRepulsionVelocity() const
{
	return LastConsumedRepulsionVelocity;
}

FVector UTsnTacticalMovementComponent::GetCachedEscapeVelocity() const
{
	return CachedEscapeVelocity;
}

void UTsnTacticalMovementComponent::SetRepulsionVelocity(const FVector& InRepulsionVelocity)
{
	// 累加语义：允许多个系统在同一帧内安全写入
	// 调试检测：仅在不同系统同帧调用时输出警告
	// （TsnStanceRepulsionSubsystem 内部已合并后只调用一次，不会触发此警告）
	ensureMsgf(
		!bHasDeferredRepulsion || DeferredRepulsionWriteFrame != GFrameCounter,
		TEXT("SetRepulsionVelocity called multiple times in frame %llu by different systems. "
			"Values will be accumulated. Check if multiple subsystems are injecting repulsion."),
		GFrameCounter);

	DeferredRepulsionVelocity += InRepulsionVelocity;

	// 仅在本周期首次写入时记录 WriteFrame，后续累加不更新。
	// UE 5.7 中 UTickableWorldSubsystem（FTickableGameObject）在所有 TickGroup
	// 执行完毕后才调度，因此 RepulsionSubsystem 天然晚于 CMC（TG_PrePhysics）；
	// 1 帧延迟由引擎调度顺序保证。首次写入保护是额外防御，
	// 防止未来 Tick 机制变更时产生同帧覆盖。
	if (!bHasDeferredRepulsion)
	{
		DeferredRepulsionWriteFrame = GFrameCounter;
	}
	bHasDeferredRepulsion = true;
}

FVector UTsnTacticalMovementComponent::GetClampedRepulsion() const
{
	float MaxRepulsionSpeed = MaxWalkSpeed * MaxRepulsionVelocityRatio;
	return DeferredRepulsionVelocity.GetClampedToMaxSize(MaxRepulsionSpeed);
}

bool UTsnTacticalMovementComponent::CanConsumeDeferredRepulsion() const
{
	return bHasDeferredRepulsion
		&& DeferredRepulsionWriteFrame < GFrameCounter
		&& !DeferredRepulsionVelocity.IsNearlyZero();
}

void UTsnTacticalMovementComponent::ConsumeDeferredRepulsion()
{
	DeferredRepulsionVelocity = FVector::ZeroVector;
	bHasDeferredRepulsion = false;
	DeferredRepulsionWriteFrame = 0;
}

bool UTsnTacticalMovementComponent::ConsumePendingEscapeRepathRequest()
{
	const bool bHadPendingRequest = bPendingEscapeRepathRequest;
	bPendingEscapeRepathRequest = false;
	return bHadPendingRequest;
}

bool UTsnTacticalMovementComponent::ShouldEnterEscapeMode(
	const FVector& RequestedVelocity2D,
	const FVector& Repulsion2D) const
{
	if (!bLastFrameInsideEscapeEnterZone)
	{
		return false;
	}

	const float RequestedSpeed = RequestedVelocity2D.Size();
	const float RepulsionSpeed = Repulsion2D.Size();
	const float CurrentSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();

	if (RequestedSpeed < EscapeModeRequestedSpeedThreshold)
	{
		return false;
	}

	if (RepulsionSpeed < MaxWalkSpeed * EscapeModeMinRepulsionRatio)
	{
		return false;
	}

	if (CurrentSpeed > EscapeModeStuckSpeedThreshold)
	{
		return false;
	}

	const float OppositionDot = FVector::DotProduct(
		RequestedVelocity2D.GetSafeNormal(),
		Repulsion2D.GetSafeNormal());
	return OppositionDot <= EscapeModeOppositionDotThreshold;
}

FVector UTsnTacticalMovementComponent::ComputeEscapeVelocity(
	const FVector& RequestedVelocity2D,
	const FVector& EscapeDirection) const
{
	if (EscapeDirection.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const float EscapeSpeedCap = MaxWalkSpeed * EscapeModeSpeedRatio;
	const float BaseEscapeSpeed = EscapeSpeedCap * 0.85f;
	FVector EscapeVelocity = EscapeDirection * BaseEscapeSpeed;

	if (!RequestedVelocity2D.IsNearlyZero())
	{
		FVector TangentDirection = FVector::CrossProduct(FVector::UpVector, EscapeDirection);
		if (FVector::DotProduct(TangentDirection, RequestedVelocity2D) < 0.f)
		{
			TangentDirection *= -1.f;
		}

		const float TangentSpeed = FMath::Max(
			0.f,
			FVector::DotProduct(RequestedVelocity2D, TangentDirection));
		const float TangentCarrySpeed = FMath::Min(
			TangentSpeed * EscapeModeTangentCarryRatio,
			EscapeSpeedCap * 0.5f);
		EscapeVelocity += TangentDirection * TangentCarrySpeed;

		const float OutwardRequestedSpeed = FMath::Max(
			0.f,
			FVector::DotProduct(RequestedVelocity2D, EscapeDirection));
		EscapeVelocity += EscapeDirection * FMath::Min(
			OutwardRequestedSpeed * 0.25f,
			EscapeSpeedCap * 0.15f);
	}

	return EscapeVelocity.GetClampedToMaxSize(EscapeSpeedCap);
}

void UTsnTacticalMovementComponent::UpdateEscapeMode(float DeltaTime)
{
	const FVector RequestedVelocity2D = FVector(
		GetLastUpdateRequestedVelocity().X,
		GetLastUpdateRequestedVelocity().Y,
		0.f);
	const FVector RecentRepulsion2D = LastConsumedRepulsionFrame == GFrameCounter
		? FVector(LastConsumedRepulsionVelocity.X, LastConsumedRepulsionVelocity.Y, 0.f)
		: FVector::ZeroVector;
	bLastFrameInsideEscapeEnterZone = false;
	bLastFrameInsideEscapeReleaseZone = false;

	FVector EscapeDirection = !RecentRepulsion2D.IsNearlyZero()
		? RecentRepulsion2D.GetSafeNormal2D()
		: FVector::ZeroVector;

	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const UCapsuleComponent* CapsuleComp = OwnerCharacter
		? OwnerCharacter->GetCapsuleComponent()
		: nullptr;
	const float MovingUnitRadius = CapsuleComp
		? CapsuleComp->GetScaledCapsuleRadius()
		: 0.f;

	if (CachedRepulsionSubsystem && GetOwner())
	{
		const FVector MyLoc = GetOwner()->GetActorLocation();
		FVector EscapeDirectionAccumulator = EscapeDirection;

		for (const FTsnStanceObstacle& Obs : CachedRepulsionSubsystem->GetStanceUnits())
		{
			if (!Obs.Unit.IsValid() || Obs.Unit.Get() == GetOwner())
			{
				continue;
			}

			const FVector Diff = MyLoc - Obs.Unit->GetActorLocation();
			const float Dist2D = Diff.Size2D();
			if (Dist2D <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const float EffectiveNavModifierRadius = Obs.NavModifierRadius + MovingUnitRadius;
			const float EnterRadius = EffectiveNavModifierRadius + EscapeModeEnterMargin;
			const float ReleaseRadius = EffectiveNavModifierRadius + EscapeModeReleaseMargin;
			const float InfluenceRadius = Obs.RepulsionRadius + MovingUnitRadius + EscapeModeReleaseMargin;

			if (Dist2D <= EnterRadius)
			{
				bLastFrameInsideEscapeEnterZone = true;
			}

			if (Dist2D <= ReleaseRadius)
			{
				bLastFrameInsideEscapeReleaseZone = true;
			}

			if (Dist2D > InfluenceRadius)
			{
				continue;
			}

			const float InfluenceRange = FMath::Max(
				InfluenceRadius - EffectiveNavModifierRadius,
				KINDA_SMALL_NUMBER);
			const float InfluenceAlpha = 1.f - FMath::Clamp(
				(Dist2D - EffectiveNavModifierRadius) / InfluenceRange,
				0.f,
				1.f);
			const float InfluenceWeight = 0.25f + InfluenceAlpha * 0.75f;
			EscapeDirectionAccumulator += Diff.GetSafeNormal2D() * InfluenceWeight;
		}

		if (!EscapeDirectionAccumulator.IsNearlyZero())
		{
			EscapeDirection = EscapeDirectionAccumulator.GetSafeNormal2D();
		}
	}

	if (bEscapeModeActive)
	{
		EscapeModeTimeRemaining = FMath::Max(0.f, EscapeModeTimeRemaining - DeltaTime);
		if (!EscapeDirection.IsNearlyZero())
		{
			CachedEscapeDirection = EscapeDirection;
		}

		CachedEscapeVelocity = ComputeEscapeVelocity(
			RequestedVelocity2D,
			CachedEscapeDirection);

		if (!CachedEscapeVelocity.IsNearlyZero())
		{
			Velocity.X = CachedEscapeVelocity.X;
			Velocity.Y = CachedEscapeVelocity.Y;
		}

#if ENABLE_DRAW_DEBUG
		if (bDrawDebugRepulsion && GetOwner() && !CachedEscapeVelocity.IsNearlyZero())
		{
			const FVector Base = GetOwner()->GetActorLocation();
			DrawDebugDirectionalArrow(
				GetWorld(), Base,
				Base + CachedEscapeVelocity.GetClampedToMaxSize(100.f),
				20.f, FColor::Magenta, false, -1.f, 0, 3.f);
		}
#endif

		if (!bLastFrameInsideEscapeReleaseZone && EscapeModeTimeRemaining <= 0.f)
		{
			++EscapeModeClearFrameCount;
		}
		else
		{
			EscapeModeClearFrameCount = 0;
		}

		if (EscapeModeClearFrameCount >= EscapeModeClearFrames)
		{
			bEscapeModeActive = false;
			bPendingEscapeRepathRequest = RequestedVelocity2D.Size()
				>= EscapeModeRequestedSpeedThreshold;
			EscapeModeTimeRemaining = 0.f;
			EscapeModeTriggerFrameCount = 0;
			EscapeModeClearFrameCount = 0;
			UE_LOG(LogTireflySquadNav, Log,
				TEXT("EscapeMode Exit: Pawn=%s PendingRepath=%s RequestedSpeed=%.1f ReleaseZone=%s"),
				*GetNameSafe(GetOwner()),
				bPendingEscapeRepathRequest ? TEXT("true") : TEXT("false"),
				RequestedVelocity2D.Size(),
				bLastFrameInsideEscapeReleaseZone ? TEXT("true") : TEXT("false"));
			CachedEscapeDirection = FVector::ZeroVector;
			CachedEscapeVelocity = FVector::ZeroVector;
		}
		return;
	}

	if (ShouldEnterEscapeMode(RequestedVelocity2D, RecentRepulsion2D))
	{
		++EscapeModeTriggerFrameCount;
		if (!EscapeDirection.IsNearlyZero())
		{
			CachedEscapeDirection = EscapeDirection;
		}

		if (EscapeModeTriggerFrameCount >= EscapeModeTriggerFrames
			&& !CachedEscapeDirection.IsNearlyZero())
		{
			bEscapeModeActive = true;
			EscapeModeTimeRemaining = EscapeModeDuration;
			EscapeModeClearFrameCount = 0;
			CachedEscapeVelocity = ComputeEscapeVelocity(
				RequestedVelocity2D,
				CachedEscapeDirection);
			Velocity.X = CachedEscapeVelocity.X;
			Velocity.Y = CachedEscapeVelocity.Y;

			UE_LOG(LogTireflySquadNav, Log,
				TEXT("EscapeMode Enter: Pawn=%s RequestedSpeed=%.1f CurrentSpeed=%.1f RepulsionSpeed=%.1f EnterZone=%s"),
				*GetNameSafe(GetOwner()),
				RequestedVelocity2D.Size(),
				FVector(Velocity.X, Velocity.Y, 0.f).Size(),
				RecentRepulsion2D.Size(),
				bLastFrameInsideEscapeEnterZone ? TEXT("true") : TEXT("false"));

#if ENABLE_DRAW_DEBUG
			if (bDrawDebugRepulsion && GetOwner() && !CachedEscapeVelocity.IsNearlyZero())
			{
				const FVector Base = GetOwner()->GetActorLocation();
				DrawDebugDirectionalArrow(
					GetWorld(), Base,
					Base + CachedEscapeVelocity.GetClampedToMaxSize(100.f),
					20.f, FColor::Magenta, false, -1.f, 0, 3.f);
			}
#endif
		}
	}
	else
	{
		EscapeModeTriggerFrameCount = 0;
		if (!bLastFrameInsideEscapeReleaseZone)
		{
			CachedEscapeDirection = FVector::ZeroVector;
		}
	}

}

void UTsnTacticalMovementComponent::UpdateStationaryLog(float DeltaTime)
{
	if (!GetOwner())
	{
		return;
	}

	const FVector CurrentLocation = GetOwner()->GetActorLocation();
	const FVector CurrentVelocity2D = FVector(Velocity.X, Velocity.Y, 0.f);
	const FVector RequestedVelocity2D = FVector(
		GetLastUpdateRequestedVelocity().X,
		GetLastUpdateRequestedVelocity().Y,
		0.f);
	const FVector RepulsionVelocity2D = FVector(
		LastConsumedRepulsionVelocity.X,
		LastConsumedRepulsionVelocity.Y,
		0.f);
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const bool bIsInStanceMode = OwnerPawn && OwnerPawn->Implements<UTsnTacticalUnit>()
		? ITsnTacticalUnit::Execute_IsInStanceMode(GetOwner())
		: false;
	if (bIsInStanceMode)
	{
		StationaryAnchorLocation = CurrentLocation;
		StationaryTime = 0.f;
		bHasLoggedStationary = false;
		return;
	}

	const AAIController* AICon = OwnerPawn
		? Cast<AAIController>(OwnerPawn->GetController())
		: nullptr;
	const UPathFollowingComponent* PathComp = AICon
		? AICon->GetPathFollowingComponent()
		: nullptr;
	const FString PathStatusDesc = PathComp
		? PathComp->GetStatusDesc()
		: TEXT("NoPathFollowingComponent");
	const bool bHasValidPath = PathComp && PathComp->HasValidPath();
	const float TravelDistance = FVector::Dist2D(CurrentLocation, StationaryAnchorLocation);
	const bool bIsStationary = CurrentVelocity2D.Size() <= StationaryLogSpeedThreshold
		&& TravelDistance <= StationaryLogDistanceTolerance;

	if (!bIsStationary)
	{
		StationaryAnchorLocation = CurrentLocation;
		StationaryTime = 0.f;
		bHasLoggedStationary = false;
		return;
	}

	StationaryTime += DeltaTime;
	if (bHasLoggedStationary || StationaryTime < StationaryLogThreshold)
	{
		return;
	}

	bHasLoggedStationary = true;
	const uint64 RepulsionAgeFrames = LastConsumedRepulsionFrame > 0
		? (GFrameCounter - LastConsumedRepulsionFrame)
		: 0;
	UE_LOG(LogTireflySquadNav, Warning,
		TEXT("StationaryUnit: Pawn=%s Time=%.2fs Speed=%.1f RequestedSpeed=%.1f RepulsionSpeed=%.1f StanceMode=%s MoveStatus=%d PathStatus=%s HasValidPath=%s EscapeActive=%s EscapeRemain=%.2f EnterZone=%s ReleaseZone=%s PendingRepath=%s RepulsionAgeFrames=%llu FinalVel=(%.1f,%.1f) RequestedVel=(%.1f,%.1f) RepulsionVel=(%.1f,%.1f) EscapeVel=(%.1f,%.1f)"),
		*GetNameSafe(GetOwner()),
		StationaryTime,
		CurrentVelocity2D.Size(),
		RequestedVelocity2D.Size(),
		RepulsionVelocity2D.Size(),
		bIsInStanceMode ? TEXT("true") : TEXT("false"),
		AICon ? static_cast<int32>(AICon->GetMoveStatus()) : -1,
		*PathStatusDesc,
		bHasValidPath ? TEXT("true") : TEXT("false"),
		bEscapeModeActive ? TEXT("true") : TEXT("false"),
		EscapeModeTimeRemaining,
		bLastFrameInsideEscapeEnterZone ? TEXT("true") : TEXT("false"),
		bLastFrameInsideEscapeReleaseZone ? TEXT("true") : TEXT("false"),
		bPendingEscapeRepathRequest ? TEXT("true") : TEXT("false"),
		RepulsionAgeFrames,
		CurrentVelocity2D.X,
		CurrentVelocity2D.Y,
		RequestedVelocity2D.X,
		RequestedVelocity2D.Y,
		RepulsionVelocity2D.X,
		RepulsionVelocity2D.Y,
		CachedEscapeVelocity.X,
		CachedEscapeVelocity.Y);
}

void UTsnTacticalMovementComponent::CalcVelocity(
	float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// ① 父类正常计算（路径跟随/Crowd 的 RequestedVelocity 都在这里被处理）
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);

	// ② 父类计算完成后叠加"上一帧写入"的排斥力
	if (CanConsumeDeferredRepulsion())
	{
		const FVector BaseVelocity = Velocity;
		const FVector BaseVelocity2D(BaseVelocity.X, BaseVelocity.Y, 0.f);
		const float SpeedCap = MaxWalkSpeed * OverspeedToleranceRatio;
		const FVector ClampedRepulsion = GetClampedRepulsion();
		LastConsumedRepulsionVelocity = ClampedRepulsion;
		LastConsumedRepulsionFrame = GFrameCounter;
		Velocity += ClampedRepulsion;

		if (Velocity.Size2D() > SpeedCap)
		{
			Velocity = Velocity.GetSafeNormal2D() * SpeedCap
				+ FVector(0.f, 0.f, Velocity.Z);
		}

#if ENABLE_DRAW_DEBUG
		if (bDrawDebugRepulsion && GetOwner())
		{
			FVector Base = GetOwner()->GetActorLocation();
			DrawDebugDirectionalArrow(
				GetWorld(), Base,
				Base + BaseVelocity2D.GetClampedToMaxSize(100.f),
				20.f, FColor::Cyan, false, -1.f, 0, 1.5f);
			DrawDebugDirectionalArrow(
				GetWorld(), Base,
				Base + ClampedRepulsion.GetClampedToMaxSize(100.f),
				20.f, FColor::Orange, false, -1.f, 0, 2.f);
			DrawDebugDirectionalArrow(
				GetWorld(), Base,
				Base + FVector(Velocity.X, Velocity.Y, 0.f).GetClampedToMaxSize(100.f),
				20.f, FColor::Green, false, -1.f, 0, 2.5f);
		}
#endif

		ConsumeDeferredRepulsion();
	}

	// ③ 第三层防线：在外层感知带内渐进削弱向内速度，进入内层后彻底禁止继续硬挤
	ApplyStancePenetrationGuard();

	// ④ EscapeMode：当路径请求仍顶向站桩障碍、而最终速度已接近 0 时，
	// 短时间内切换为排斥主导的逃逸速度，避免单位长期卡在原地。
	UpdateEscapeMode(DeltaTime);
	UpdateStationaryLog(DeltaTime);
}

void UTsnTacticalMovementComponent::ApplyStancePenetrationGuard()
{
	if (!CachedRepulsionSubsystem) return;

	const TArray<FTsnStanceObstacle>& StanceUnits = CachedRepulsionSubsystem->GetStanceUnits();
	if (StanceUnits.Num() == 0) return;

	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const UCapsuleComponent* CapsuleComp = OwnerCharacter
		? OwnerCharacter->GetCapsuleComponent()
		: nullptr;
	const float MovingUnitRadius = CapsuleComp
		? CapsuleComp->GetScaledCapsuleRadius()
		: 0.f;

	FVector MyLoc = GetOwner()->GetActorLocation();
	FVector Vel2D = FVector(Velocity.X, Velocity.Y, 0.f);

	// 每帧通过 Obs.Unit->GetActorLocation() 动态读取位置（FTsnStanceObstacle 不缓存位置快照）。
	for (const FTsnStanceObstacle& Obs : StanceUnits)
	{
		if (!Obs.Unit.IsValid()) continue;

		FVector Diff = MyLoc - Obs.Unit->GetActorLocation();
		float Dist2D = Diff.Size2D();
		const float EffectiveNavModifierRadius = Obs.NavModifierRadius + MovingUnitRadius;
		const float EffectiveRepulsionRadius = Obs.RepulsionRadius + MovingUnitRadius;

		if (Dist2D >= EffectiveRepulsionRadius || Dist2D <= KINDA_SMALL_NUMBER) continue;

		float InwardClipScale = 0.f;
		if (Dist2D < EffectiveNavModifierRadius)
		{
			// 内层：完全禁止继续朝站桩单位推进
			InwardClipScale = 1.f;
		}
		else
		{
			// 外层：越接近 ObstacleRadius，越强地削弱朝向站桩单位的前进分量。
			const float OuterRange = FMath::Max(
				EffectiveRepulsionRadius - EffectiveNavModifierRadius,
				KINDA_SMALL_NUMBER);
			const float OuterAlpha = 1.f
				- ((Dist2D - EffectiveNavModifierRadius) / OuterRange);
			InwardClipScale = OuterBandInwardClipRatio * FMath::Clamp(OuterAlpha, 0.f, 1.f);
		}

		if (InwardClipScale <= KINDA_SMALL_NUMBER) continue;

		FVector TowardStance2D = -Diff.GetSafeNormal2D();
		float InwardComponent = FVector::DotProduct(Vel2D, TowardStance2D);

		if (InwardComponent > 0.f)
		{
			const float ClippedComponent = InwardComponent * InwardClipScale;
			Vel2D -= TowardStance2D * ClippedComponent;

			UE_LOG(LogTireflySquadNav, Verbose,
				TEXT("StancePenetrationGuard: clipped inward velocity (%.1f / %.1f cm/s, scale=%.2f) toward %s (dist2D=%.1f, ObstacleR=%.1f, RepulsionR=%.1f, MovingR=%.1f)"),
				ClippedComponent, InwardComponent, InwardClipScale,
				*GetNameSafe(Obs.Unit.Get()), Dist2D,
				EffectiveNavModifierRadius, EffectiveRepulsionRadius, MovingUnitRadius);
		}
	}

	Velocity.X = Vel2D.X;
	Velocity.Y = Vel2D.Y;
}
