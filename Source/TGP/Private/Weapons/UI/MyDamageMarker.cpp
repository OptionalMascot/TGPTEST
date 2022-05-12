// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/UI/MyDamageMarker.h"

#include "Components/SceneComponent.h"

// Sets default values
AMyDamageMarker::AMyDamageMarker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(_root);
	
	//_widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	//_widget->SetupAttachment(_root);
}

// Called when the game starts or when spawned
void AMyDamageMarker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyDamageMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

