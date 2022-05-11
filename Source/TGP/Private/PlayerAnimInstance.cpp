// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

void UPlayerAnimInstance::UpdateAnimationVariables()
{
	if(!PlayerPawn)
	{
		PlayerPawn = TryGetPawnOwner();
	}
	else
	{
		MoveSpeed = FVector(PlayerPawn->GetVelocity().X, PlayerPawn->GetVelocity().Y, 0.0f).Size();
	}
}
