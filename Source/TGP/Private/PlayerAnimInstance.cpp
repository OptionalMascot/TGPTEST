// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "Aki/AkiCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::UpdateAnimationVariables()
{
	if(!PlayerPawn)
	{
		PlayerPawn = TryGetPawnOwner();
		Player = Cast<AAkiCharacter>(PlayerPawn);
	}
	else
	{
		MoveSpeed = FVector(PlayerPawn->GetVelocity().X, PlayerPawn->GetVelocity().Y, 0.0f).Size();
		if(Player)
		{
			IsSprinitng = Player->IsSprinting;
			IsJumping = !Player->GetCharacterMovement()->IsMovingOnGround();
			IsAiming = Player->IsAiming;
		}
	}
}
