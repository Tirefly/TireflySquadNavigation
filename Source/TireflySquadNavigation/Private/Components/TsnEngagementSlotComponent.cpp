// TsnEngagementSlotComponent.cpp

#include "Components/TsnEngagementSlotComponent.h"
#include "Subsystems/TsnEngagementSlotSubsystem.h"
#include "DrawDebugHelpers.h"
#include "TsnLog.h"

UTsnEngagementSlotComponent::UTsnEngagementSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UTsnEngagementSlotComponent::RequestSlot(AActor* Requester, float AttackRange)
{
	if (!Requester || !GetOwner())
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	CleanupInvalidSlots();

	// 通过子系统检查：若请求者已在其他目标上占有槽位，自动释放旧槽位
	if (UWorld* World = GetWorld())
	{
		if (auto* SlotSS = World->GetSubsystem<UTsnEngagementSlotSubsystem>())
		{
			UTsnEngagementSlotComponent* OldSlotComp = SlotSS->FindOccupiedSlotComponent(Requester);
			if (OldSlotComp && OldSlotComp != this)
			{
				OldSlotComp->ReleaseSlot(Requester);
			}
		}
	}

	// 已有槽位 → 检查攻击距离是否发生变化
	if (FTsnEngagementSlotInfo* Existing = FindExistingSlot(Requester))
	{
		float NewRadius = FMath::Max(AttackRange + SlotRadiusOffset, 10.f);
		if (FMath::Abs(Existing->Radius - NewRadius) > SameRingRadiusTolerance)
		{
			// 攻击距离变化超过容差 → 释放旧槽位，重新分配
			ReleaseSlot(Requester);
		}
		else
		{
			// 复用本地极坐标，重新计算当前世界快照
			return CalculateWorldPosition(Existing->AngleDeg, Existing->Radius);
		}
	}

	// 槽位已满 → 面向请求者方向的合理位置（不阻塞攻击）
	if (Slots.Num() >= MaxSlots)
	{
		float Radius = FMath::Max(AttackRange + SlotRadiusOffset, 10.f);
		FVector Dir = (Requester->GetActorLocation()
			- GetOwner()->GetActorLocation()).GetSafeNormal2D();
		float FallbackAngle = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		return CalculateWorldPosition(FallbackAngle, Radius);
	}

	// 计算环半径和理想角度
	float Radius = FMath::Max(AttackRange + SlotRadiusOffset, 10.f);
	FVector Dir = (Requester->GetActorLocation()
		- GetOwner()->GetActorLocation()).GetSafeNormal2D();
	float IdealAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));

	// 找不冲突的空位角度
	float BestAngle = FindUnoccupiedAngle(IdealAngleDeg, Radius);

	// 注册槽位并向子系统登记
	FTsnEngagementSlotInfo NewSlot;
	NewSlot.Occupant = Requester;
	NewSlot.AngleDeg = BestAngle;
	NewSlot.Radius = Radius;
	Slots.Add(NewSlot);

	if (UWorld* World = GetWorld())
	{
		if (auto* SlotSS = World->GetSubsystem<UTsnEngagementSlotSubsystem>())
		{
			SlotSS->RegisterSlotOccupancy(Requester, this);
		}
	}

	return CalculateWorldPosition(BestAngle, Radius);
}

void UTsnEngagementSlotComponent::ReleaseSlot(AActor* Requester)
{
	int32 Removed = Slots.RemoveAll([Requester](const FTsnEngagementSlotInfo& Slot)
	{
		return Slot.Occupant == Requester;
	});

	if (Removed > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (auto* SlotSS = World->GetSubsystem<UTsnEngagementSlotSubsystem>())
			{
				SlotSS->UnregisterSlotOccupancy(Requester);
			}
		}
	}
}

bool UTsnEngagementSlotComponent::HasSlot(AActor* Requester) const
{
	for (const FTsnEngagementSlotInfo& Slot : Slots)
	{
		if (Slot.Occupant == Requester) return true;
	}
	return false;
}

int32 UTsnEngagementSlotComponent::GetOccupiedSlotCount() const
{
	int32 Count = 0;
	for (const FTsnEngagementSlotInfo& Slot : Slots)
	{
		if (Slot.Occupant.IsValid()) ++Count;
	}
	return Count;
}

bool UTsnEngagementSlotComponent::IsSlotAvailable(AActor* Requester) const
{
	if (HasSlot(Requester)) return true;
	return Slots.Num() < MaxSlots;
}

float UTsnEngagementSlotComponent::FindUnoccupiedAngle(
	float IdealAngleDeg, float Radius) const
{
	// 收集同环上已被占据的角度
	TArray<float> OccupiedAngles;
	for (const FTsnEngagementSlotInfo& Slot : Slots)
	{
		if (!Slot.Occupant.IsValid()) continue;
		if (FMath::Abs(Slot.Radius - Radius) < SameRingRadiusTolerance)
		{
			OccupiedAngles.Add(Slot.AngleDeg);
		}
	}

	if (OccupiedAngles.Num() == 0)
	{
		return IdealAngleDeg;
	}

	// 基于半径计算最小角度间隔（弧长 = 半径 × 弧度）
	float MinAngularGapDeg = FMath::RadiansToDegrees(MinSlotSpacing / Radius);
	MinAngularGapDeg = FMath::Max(MinAngularGapDeg, 25.f);

	// 从理想角度出发，正负交替搜索空位
	float SearchStep = MinAngularGapDeg * 0.5f;
	for (float Offset = 0.f; Offset <= 180.f; Offset += SearchStep)
	{
		float Candidates[2] = { IdealAngleDeg + Offset, IdealAngleDeg - Offset };
		int32 NumCandidates = (Offset == 0.f) ? 1 : 2;

		for (int32 i = 0; i < NumCandidates; ++i)
		{
			bool bConflict = false;
			for (float Occupied : OccupiedAngles)
			{
				if (FMath::Abs(FMath::FindDeltaAngleDegrees(Candidates[i], Occupied))
					< MinAngularGapDeg)
				{
					bConflict = true;
					break;
				}
			}
			if (!bConflict) return Candidates[i];
		}
	}

	// 环被占满 → 返回理想角度（不阻塞攻击）
	return IdealAngleDeg;
}

FVector UTsnEngagementSlotComponent::CalculateWorldPosition(
	float AngleDeg, float Radius) const
{
	// AngleDeg 为世界空间方位角（0° = +X 轴），不随目标旋转
	float AngleRad = FMath::DegreesToRadians(AngleDeg);
	FVector OwnerLoc = GetOwner()->GetActorLocation();
	return OwnerLoc + FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f) * Radius;
}

void UTsnEngagementSlotComponent::CleanupInvalidSlots()
{
	Slots.RemoveAll([](const FTsnEngagementSlotInfo& Slot) {
		return !Slot.Occupant.IsValid();
	});
}

void UTsnEngagementSlotComponent::OnOwnerReleased()
{
	// 双层 null-guard：GetWorld() / GetSubsystem() 可能为 null
	if (UWorld* World = GetWorld())
	{
		if (auto* SlotSS = World->GetSubsystem<UTsnEngagementSlotSubsystem>())
		{
			for (const FTsnEngagementSlotInfo& Slot : Slots)
			{
				if (Slot.Occupant.IsValid())
				{
					SlotSS->UnregisterSlotOccupancy(Slot.Occupant.Get());
				}
			}
		}
	}

	Slots.Empty();
}

FTsnEngagementSlotInfo* UTsnEngagementSlotComponent::FindExistingSlot(AActor* Requester)
{
	for (FTsnEngagementSlotInfo& Slot : Slots)
	{
		if (Slot.Occupant == Requester) return &Slot;
	}
	return nullptr;
}

#if ENABLE_DRAW_DEBUG
void UTsnEngagementSlotComponent::DrawDebugSlots() const
{
	if (!bDrawDebugSlots || !GetOwner()) return;
	FVector OwnerLoc = GetOwner()->GetActorLocation();

	for (const FTsnEngagementSlotInfo& Slot : Slots)
	{
		FVector SlotPos = CalculateWorldPosition(Slot.AngleDeg, Slot.Radius);
		FColor Color = Slot.Occupant.IsValid() ? FColor::Red : FColor::Green;
		DrawDebugSphere(GetOwner()->GetWorld(), SlotPos, 15.f, 8, Color, false, -1.f);
		DrawDebugLine(GetOwner()->GetWorld(), OwnerLoc, SlotPos,
			FColor(100, 100, 100), false, -1.f, 0, 0.5f);
	}

	TSet<int32> DrawnRadii;
	for (const FTsnEngagementSlotInfo& Slot : Slots)
	{
		int32 RadiusKey = FMath::RoundToInt(Slot.Radius);
		if (!DrawnRadii.Contains(RadiusKey))
		{
			DrawnRadii.Add(RadiusKey);
			DrawDebugCircle(GetOwner()->GetWorld(), OwnerLoc, Slot.Radius, 32,
				FColor::Cyan, false, -1.f, 0, 0.5f,
				FVector::RightVector, FVector::ForwardVector, false);
		}
	}
}
#endif
