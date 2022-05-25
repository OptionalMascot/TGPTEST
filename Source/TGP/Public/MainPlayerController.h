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
	void InitHUD();

	UFUNCTION(BlueprintImplementableEvent)
	void HideCrosshair(bool Hide);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD", meta=(AllowPrivateAccess = true))
	TSubclassOf<class UUserWidget> HUDWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD", meta=(AllowPrivateAccess = true))
	UUserWidget* HUD;
};
