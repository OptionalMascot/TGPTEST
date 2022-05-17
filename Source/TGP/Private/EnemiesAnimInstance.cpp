// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAnimInstance.h"

#include "Ai/BaseAiCharacter.h"

void UEnemiesAnimInstance::UpdateAnimationVariables()
{
	if(!Zombie)
	{
		Zombie = Cast<ABaseAiCharacter>(TryGetPawnOwner());
	}
}
