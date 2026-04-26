// TsnEngagementSlotSubsystem.cpp

#include "Subsystems/TsnEngagementSlotSubsystem.h"
#include "Components/TsnEngagementSlotComponent.h"

void UTsnEngagementSlotSubsystem::RegisterSlotOccupancy(
	AActor* Requester, UTsnEngagementSlotComponent* SlotComp)
{
	if (!Requester || !SlotComp) return;
	SlotOccupancyMap.Add(Requester, SlotComp);
}

void UTsnEngagementSlotSubsystem::UnregisterSlotOccupancy(AActor* Requester)
{
	if (!Requester) return;
	SlotOccupancyMap.Remove(Requester);
}

UTsnEngagementSlotComponent* UTsnEngagementSlotSubsystem::FindOccupiedSlotComponent(
	AActor* Requester) const
{
	if (!Requester) return nullptr;
	if (const TWeakObjectPtr<UTsnEngagementSlotComponent>* Found = SlotOccupancyMap.Find(Requester))
	{
		return Found->Get();
	}
	return nullptr;
}
