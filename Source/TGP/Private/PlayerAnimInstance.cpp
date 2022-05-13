// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AkiPlayer.h"

void UPlayerAnimInstance::UpdateAnimationVariables()
{
	if(!PlayerPawn)
	{
		PlayerPawn = TryGetPawnOwner();
		Player = Cast<AAkiPlayer>(PlayerPawn);
	}
	else
	{
		MoveSpeed = FVector(PlayerPawn->GetVelocity().X, PlayerPawn->GetVelocity().Y, 0.0f).Size();
		if(Player)
		{
			IsJumping = !Player->GetCharacterMovement()->IsMovingOnGround();
			IsSprinting = Player->IsSprinting;
		}
	}
}
