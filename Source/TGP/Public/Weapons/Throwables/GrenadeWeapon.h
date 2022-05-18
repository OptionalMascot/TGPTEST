// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/Interfaces/WeaponInterfaces.h"
#include "NiagaraComponent.h"
#include "Weapons/Throwables/ThrowableWeapon.h"
#include "GrenadeWeapon.generated.h"

class UThrowableInfo;
class APlayerController;

UCLASS()
class TGP_API AGrenadeWeapon : public AThrowableWeapon, public IWaitTimer
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) USkeletalMeshComponent* ItemSkeletalMesh;

	// Particle System
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UNiagaraComponent* _particleSystem;
	
	bool _startFuse;
	virtual void StartWaitTimer(AActor* actor, float time) override;
	virtual void ExplodeGrenade();
	void SphereCastForTargets();
public:	
	// Called every frame
	void DestroyObj();
	virtual void Tick(float DeltaTime) override;
	void Initialize(UThrowableInfo* throwableInfo) override;
};
