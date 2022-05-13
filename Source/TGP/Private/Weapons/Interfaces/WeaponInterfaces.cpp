// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Interfaces/WeaponInterfaces.h"

#include <Actor.h>

#include "Item/BaseItem.h"


// Add default functionality here for any IWeaponInterfaces functions that are not pure virtual.


bool IHealth::AdjustHealth(float damage)
{
	health -= damage;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Hit! Health: ") + FString::FromInt(health));
	if(health <= 0)
	{
		dead = true;
		KillObject();
	}
	return dead;
}

bool ICanHitScan::DoRaycastReturnResult(UWorld* world, FHitResult& result, FVector startPoint, FVector endPoint, ECollisionChannel collisionChannel)
{
	bool DidTrace = world->LineTraceSingleByChannel(
			result,		//result
			startPoint,		//start
			endPoint,		//end
			collisionChannel	//collision channel
			);
	
	return DidTrace;
}


bool IHasAmmo::TryUseAmmo(AActor* actor, int amount)
{
	if(currentAmmoClip > 0)
	{
		if(reloading)
		{
			reloading = false;
			CancelReload(actor);
		}
		currentAmmoClip -= amount;
		// Set Ammo in "Weapon Item" to the new amount
		return true;
	}
	else
	{
		return false;
	}
}


void IHasAmmo::StartReloadAmmo(AActor* actor)
{
	
	if(!reloading)
	{
		reloading = true;
		UWorld* world = actor->GetWorld();
		ReloadEnded(); // DELETE WHEN TRYING TO FIX TIMER
		
		//FTimerDynamicDelegate eventTest;
		//eventTest.BindDynamic(this, &IHasAmmo::ReloadEnded);
		//world->GetTimerManager().SetTimer(reloadTimerHandler, eventTest, 5, true);
	}
}

void IHasAmmo::TryReload(AActor* actor)
{
	if(currentReserves > 0)
	{
		StartReloadAmmo(actor);
	}
}

void IHasAmmo::ReloadEnded()
{
	if(reloading)
	{
		int neededClip = maxAmmo - currentAmmoClip;
		int newClip = FMath::Min(maxAmmo, currentReserves);
		int newValue = FMath::Min(newClip, neededClip);
		int addedBack = currentAmmoClip + newValue;
		if(!infiniteReserves)
		{
			currentReserves -= newValue;
		}

		currentAmmoClip = addedBack;
		reloading = false;


		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("WeaponReloaded"));
		// Set ammo in "WeaponItem"
	}
}

void IHasAmmo::CancelReload(AActor* actor)
{
	
}

void IUseRecoil::ApplyRecoilPitch(APlayerController* controller, float value)
{
	if(controller)
		controller->AddPitchInput(value);
}

void IUseRecoil::ApplyRecoilYaw(APlayerController* controller, float value)
{
	if(controller)
		controller->AddYawInput(value);
}
