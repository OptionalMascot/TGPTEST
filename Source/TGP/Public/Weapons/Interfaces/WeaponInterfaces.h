// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "UObject/Interface.h"
#include "Components/TimelineComponent.h"
#include "WeaponInterfaces.generated.h"

class UGunItem;

UINTERFACE(MinimalAPI)
class UCanHitScan : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API ICanHitScan
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
	bool DoRaycastReturnResult(UWorld* world, FHitResult& result, FVector startPoint, FVector endPoint, ECollisionChannel collisionChannel);
	
};

UINTERFACE(MinimalAPI)
class UHasAmmo : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API IHasAmmo
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
	int maxAmmo;
	int currentAmmoClip;
	int currentReserves;

	bool reloading;
	float reloadTime;

	bool infiniteReserves;

	FTimerHandle reloadTimerHandler;
	
	bool TryUseAmmo(AActor* actor, int amount);
	UFUNCTION()	virtual void StartReloadAmmo(AActor* actor);
	void TryReload(AActor* actor);
	UFUNCTION() virtual void ReloadEnded();
	void SetMaxAmmo(int ammo) { maxAmmo = ammo;}
	int GetAmmoCount() { return currentAmmoClip; }
	int GetReserves() { return currentReserves; }
	void AddAmmo(int amount) { currentReserves += amount; }
	virtual void CancelReload(AActor* actor);
};

UINTERFACE(MinimalAPI)
class UWaitTimer : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API IWaitTimer
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
	FTimerHandle waitTimeHandler;
	virtual void StartWaitTimer(AActor* actor, float time) {};
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUseRecoil : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API IUseRecoil
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
	void ApplyRecoilPitch(APlayerController* controller, float value);
	void ApplyRecoilYaw(APlayerController* controller, float value);
};