// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"

#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(HUDBlueprint)
	{
		DisplayedHUD = CreateWidget(this, HUDBlueprint);
		DisplayedHUD->AddToViewport();
	}
}
