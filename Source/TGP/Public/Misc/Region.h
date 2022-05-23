// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"

#include "Region.generated.h"

UCLASS()
class TGP_API ARegion : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", Meta = (AllowPrivateAccess = true))
	UBoxComponent* regionBounds;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (AllowPrivateAccess = true))
	FString name;

	virtual void BeginPlay() override;
public:	
	ARegion();
	FVector GetRandomPointInRegion();
};
