// Fill out your copyright notice in the Description page of Project Settings.

#include "Ai/BaseAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

ABaseAIController::ABaseAIController()
{
	blackboardComponenet = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard Component"));
	AddOwnedComponent(blackboardComponenet);
}

void ABaseAIController::ChangeAIControllerStatus(bool status)
{
	blackboardComponenet->SetValueAsBool("isSpawned", status);
	
	if (status)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Green, FString::Printf(TEXT("Enemy Spawned")));
	}
}

void ABaseAIController::SetObjective(AActor* objective)
{
	blackboardComponenet->SetValueAsObject("objective",objective);
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseAIController::RunBT()
{
	RunBehaviorTree(behaviorTree);
}