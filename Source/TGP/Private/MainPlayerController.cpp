// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"

#include "../../../../../../../../Epic/UE4/UE_4.26.2/Engine/Shaders/Private/SolidColorPixelShader.ush"
#include "Blueprint/UserWidget.h"
#include "TGP/FP_FirstPerson/FP_FirstPersonCharacter.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(HUDBlueprint)
	{
		DisplayedHUD = CreateWidget(this, HUDBlueprint);
		DisplayedHUD->AddToViewport();
	}
}

void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(InPawn->IsA(AFP_FirstPersonCharacter::StaticClass()))
	{
		AFP_FirstPersonCharacter* MainPlayer = Cast<AFP_FirstPersonCharacter>(InPawn);
		MainPlayer->HUDSetUP();
	}
}
