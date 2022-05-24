// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/Region.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

ARegion::ARegion()
{
	PrimaryActorTick.bCanEverTick = true;
	regionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Region Bounds"));
	RootComponent = regionBounds;
}

void ARegion::BeginPlay()
{	
	Super::BeginPlay();
	GetRandomPointInRegion();
}

FVector ARegion::GetRandomPointInRegion()
{
	FVector boxOrigin;
	FVector boxExtent;
	float sphr; // ignore this
	UKismetSystemLibrary::GetComponentBounds(regionBounds,boxOrigin,boxExtent,sphr);
	return  UKismetMathLibrary::RandomPointInBoundingBox(boxOrigin,boxExtent);
}

