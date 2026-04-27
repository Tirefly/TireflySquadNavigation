#pragma once

#include "Kismet/GameplayStatics.h"
#include "TsnTestChessPiece.h"
#include "TsnTestSpawner.h"
#include "TsnTestTargetDummy.h"

namespace TsnTestAutomation
{
	inline constexpr TCHAR DemoAutomationMapPath[] =
		TEXT("/TireflySquadNavigation/FunctionShowcase/Map/MAP_TsnDemo_Siege");

	inline constexpr TCHAR BehaviorTreeAssetPath[] =
		TEXT("/TireflySquadNavigation/FunctionShowcase/AI/BehaviorTree/BT_Tsn_Test.BT_Tsn_Test");

	inline constexpr TCHAR BlackboardAssetPath[] =
		TEXT("/TireflySquadNavigation/FunctionShowcase/AI/BehaviorTree/BB_Tsn_Test.BB_Tsn_Test");

	inline void ResetShowcaseActors(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<AActor*> ActorsToDestroy;
		TArray<AActor*> FoundActors;

		UGameplayStatics::GetAllActorsOfClass(World, ATsnTestSpawner::StaticClass(), FoundActors);
		ActorsToDestroy.Append(FoundActors);
		FoundActors.Reset();

		UGameplayStatics::GetAllActorsOfClass(World, ATsnTestChessPiece::StaticClass(), FoundActors);
		ActorsToDestroy.Append(FoundActors);
		FoundActors.Reset();

		UGameplayStatics::GetAllActorsOfClass(World, ATsnTestTargetDummy::StaticClass(), FoundActors);
		ActorsToDestroy.Append(FoundActors);

		for (AActor* Actor : ActorsToDestroy)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy();
			}
		}
	}
}