// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/Interfaces/WeaponInterfaces.h"
#include "GrenadeWeapon.generated.h"

class UThrowableInfo;

UCLASS()
class TGP_API AGrenadeWeapon : public AActor, public IWaitTimer
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) USkeletalMeshComponent* ItemSkeletalMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UThrowableInfo* _throwableInfo;
	
	bool _startFuse;
	virtual void StartWaitTimer(AActor* actor, float time) override;
	void ExplodeGrenade();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetInitialThrowForce(FVector forceDir);
	void Initialize(UThrowableInfo* throwableInfo);
};
