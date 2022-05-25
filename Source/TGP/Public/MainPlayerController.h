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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD")
	TSubclassOf<class UUserWidget> HUDBlueprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD")
	UUserWidget* DisplayedHUD;

protected:
	virtual  void BeginPlay() override;
};
