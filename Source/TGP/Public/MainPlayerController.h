// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TGP_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void HideCrosshair();

	UFUNCTION(BlueprintImplementableEvent)
	void RevealCrosshair();

	UFUNCTION(BlueprintCallable)
	void TriggerCrossShow() { RevealCrosshair(); }
	
	UFUNCTION(BlueprintCallable)
    void TriggerCrossHide() { HideCrosshair(); }
};
