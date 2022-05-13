// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/UI/MyDamageMarker.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"
#include "Weapons/UI/UserWidgetTest.h"
#include "Components/WidgetComponent.h"

// Sets default values
AMyDamageMarker::AMyDamageMarker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(_root);
	
	_widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	_widget->SetupAttachment(_root);
}

// Called when the game starts or when spawned
void AMyDamageMarker::BeginPlay()
{
	Super::BeginPlay();
	_spawnedBy = nullptr;
	_aliveTime = 1.0f;
}

void AMyDamageMarker::WidgetBillboardAndMove(float DeltaTime)
{
	if(_spawnedBy)
	{
		FVector start =_widget->GetComponentTransform().GetLocation();
		FVector target = _spawnedBy->GetActorLocation();
		FRotator rotator = UKismetMathLibrary::FindLookAtRotation(start, target);
		SetActorRotation(rotator);
	}
	SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, 300.0f * DeltaTime));
	_aliveTime -= DeltaTime;
	if(_aliveTime <= 0.0f)
	{
		Destroy();
	}
}

// Called every frame
void AMyDamageMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WidgetBillboardAndMove(DeltaTime);
}

void AMyDamageMarker::SetText(int num)
{
	UWidgetComponent* widget = GetWidget();
	UUserWidget* userWid = widget->GetUserWidgetObject();
	UUserWidgetTest* castedWidget = Cast<UUserWidgetTest>(userWid);
	if(castedWidget)
	{
		castedWidget->SetText(10.0f);
	}
}

