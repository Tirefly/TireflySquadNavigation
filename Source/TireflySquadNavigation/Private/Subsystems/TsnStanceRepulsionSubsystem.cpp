// TsnStanceRepulsionSubsystem.cpp

#include "Subsystems/TsnStanceRepulsionSubsystem.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

// 控制台变量：运行时切换排斥力调试可视化
#if ENABLE_DRAW_DEBUG
static bool GDrawDebugTsnRepulsion = false;
static FAutoConsoleVariableRef CVarDrawDebugTsnRepulsion(
	TEXT("tsn.debug.DrawRepulsion"),
	GDrawDebugTsnRepulsion,
	TEXT("1 = Draw TsnStanceRepulsionSubsystem area rings and repulsion force arrows (Development/Debug only)"),
	ECVF_Default);
#endif

void UTsnStanceRepulsionSubsystem::RegisterStanceUnit(
	AActor* Unit, float InRepulsionRadius, float InRepulsionStrength, float InNavModifierRadius)
{
	if (!Unit) return;
	// 幂等保护：已注册则跳过
	for (const auto& Entry : StanceUnits)
	{
		if (Entry.Unit == Unit) return;
	}
	StanceUnits.Add({ Unit, InRepulsionRadius, InRepulsionStrength, InNavModifierRadius });
}

void UTsnStanceRepulsionSubsystem::UnregisterStanceUnit(AActor* Unit)
{
	// 幂等：RemoveAll 对未注册的 Actor 无副作用；同时清理失效弱引用
	StanceUnits.RemoveAll([Unit](const FTsnStanceObstacle& Obs)
	{
		return Obs.Unit == Unit || !Obs.Unit.IsValid();
	});
}

void UTsnStanceRepulsionSubsystem::UpdateStanceUnit(
	AActor* Unit, float InRepulsionRadius, float InRepulsionStrength, float InNavModifierRadius)
{
	if (!Unit) return;
	for (FTsnStanceObstacle& Entry : StanceUnits)
	{
		if (Entry.Unit == Unit)
		{
			Entry.RepulsionRadius = InRepulsionRadius;
			Entry.RepulsionStrength = InRepulsionStrength;
			Entry.NavModifierRadius = InNavModifierRadius;
			return;
		}
	}
}

void UTsnStanceRepulsionSubsystem::CleanupInvalidEntries()
{
	StanceUnits.RemoveAll([](const FTsnStanceObstacle& Obs) {
		return !Obs.Unit.IsValid();
	});
}

void UTsnStanceRepulsionSubsystem::RegisterMovingUnit(AActor* Unit)
{
	if (!Unit) return;
	MovingUnits.AddUnique(Unit);
}

void UTsnStanceRepulsionSubsystem::UnregisterMovingUnit(AActor* Unit)
{
	MovingUnits.RemoveAll([Unit](const TWeakObjectPtr<AActor>& Ptr)
	{
		return Ptr == Unit || !Ptr.IsValid();
	});
}

void UTsnStanceRepulsionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CleanupInvalidEntries();
	if (StanceUnits.Num() == 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugRepulsion || GDrawDebugTsnRepulsion)
	{
		for (const FTsnStanceObstacle& Obs : StanceUnits)
		{
			if (!Obs.Unit.IsValid()) continue;
			FVector StanceLoc = Obs.Unit->GetActorLocation();
			DrawDebugCircle(World, StanceLoc, Obs.NavModifierRadius, 32,
				FColor::Red, false, -1.f, 0, 1.f,
				FVector::RightVector, FVector::ForwardVector, false);
			DrawDebugCircle(World, StanceLoc, Obs.RepulsionRadius, 32,
				FColor::Yellow, false, -1.f, 0, 1.f,
				FVector::RightVector, FVector::ForwardVector, false);
		}
	}
#endif

	// 清理失效的移动单位
	MovingUnits.RemoveAll([](const TWeakObjectPtr<AActor>& Ptr) { return !Ptr.IsValid(); });

	// O(站桩数 × 移动数) 遍历；十几到几十单位的场景规模可接受
	for (const TWeakObjectPtr<AActor>& WeakUnit : MovingUnits)
	{
		AActor* MovingActor = WeakUnit.Get();
		if (!MovingActor) continue;

		ACharacter* MovingChar = Cast<ACharacter>(MovingActor);
		if (!MovingChar) continue;

		// 只处理移动中的战斗单位
		UTsnStanceObstacleComponent* ObsComp =
			MovingChar->FindComponentByClass<UTsnStanceObstacleComponent>();
		if (!ObsComp || ObsComp->GetMobilityStance() != ETsnMobilityStance::Moving)
			continue;

		UTsnTacticalMovementComponent* MoveComp =
			Cast<UTsnTacticalMovementComponent>(MovingChar->GetCharacterMovement());
		if (!MoveComp) continue;

		const UCapsuleComponent* CapsuleComp = MovingChar->GetCapsuleComponent();
		const float MovingUnitRadius = CapsuleComp
			? CapsuleComp->GetScaledCapsuleRadius()
			: 0.f;

		FVector MyLoc = MovingChar->GetActorLocation();
		FVector TotalRepulsion = FVector::ZeroVector;

		for (const FTsnStanceObstacle& Obs : StanceUnits)
		{
			if (!Obs.Unit.IsValid() || Obs.Unit.Get() == MovingChar) continue;

			FVector Diff = MyLoc - Obs.Unit->GetActorLocation();
			float Dist2D = Diff.Size2D();
			const float EffectiveNavModifierRadius = Obs.NavModifierRadius + MovingUnitRadius;
			const float EffectiveRepulsionRadius = Obs.RepulsionRadius + MovingUnitRadius;

			if (Dist2D >= EffectiveRepulsionRadius || Dist2D <= KINDA_SMALL_NUMBER) continue;

			// 双阶段力度计算
			float ForceMagnitude;

			if (Dist2D < EffectiveNavModifierRadius)
			{
				// 内层：已穿入 NavModifier 区域 → 全力排斥
				float Penetration = 1.f - (Dist2D / EffectiveNavModifierRadius);
				ForceMagnitude = Obs.RepulsionStrength * FMath::Square(Penetration);
			}
			else
			{
				// 外层：极微弱线性引导
				float OuterRange = EffectiveRepulsionRadius - EffectiveNavModifierRadius;
				float OuterDist = Dist2D - EffectiveNavModifierRadius;
				float OuterFactor = 1.f - (OuterDist / OuterRange);
				ForceMagnitude = Obs.RepulsionStrength * OuterRepulsionRatio * OuterFactor;
			}

			// 方向：切线引导 + 径向排斥
			FVector RadialDir = Diff.GetSafeNormal2D();
			const FVector RequestedMoveDir =
				MoveComp->GetLastUpdateRequestedVelocity().GetSafeNormal2D();
			const FVector CurrentMoveDir = !RequestedMoveDir.IsNearlyZero()
				? RequestedMoveDir
				: MoveComp->Velocity.GetSafeNormal2D();

			FVector ForceDir;
			if (!CurrentMoveDir.IsNearlyZero())
			{
				FVector Tangent = FVector::CrossProduct(FVector::UpVector, CurrentMoveDir);
				const float Side = FVector::DotProduct(RadialDir, Tangent);
				if (FMath::Abs(Side) <= TangentSideDeadZone)
				{
					// 接近正对障碍时不强行二选一绕行，避免左右切线方向每帧翻转。
					ForceDir = RadialDir;
				}
				else
				{
					const FVector TangentDir = Tangent * FMath::Sign(Side);
					ForceDir = (TangentDir * TangentBlendRatio
						+ RadialDir * (1.f - TangentBlendRatio)).GetSafeNormal();
				}
			}
			else
			{
				ForceDir = RadialDir;
			}

			TotalRepulsion += ForceDir * ForceMagnitude;
		}

		if (!TotalRepulsion.IsNearlyZero())
		{
			// 通过合法接口注入，不直接修改 Velocity
			MoveComp->SetRepulsionVelocity(TotalRepulsion * DeltaTime);

#if ENABLE_DRAW_DEBUG
			if (bDrawDebugRepulsion || GDrawDebugTsnRepulsion)
			{
				DrawDebugDirectionalArrow(
					World,
					MovingActor->GetActorLocation(),
					MovingActor->GetActorLocation() + TotalRepulsion.GetSafeNormal() * 80.f,
					20.f, FColor::Orange, false, -1.f, 0, 2.f);
			}
#endif
		}
	}
}
