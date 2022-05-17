// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemiesAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TGP_API UEnemiesAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationVariables();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class ABaseAiCharacter* Zombie;
	
};
