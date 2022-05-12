// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitscanWeaponComponent.h"
#include "Weapons/UI/MyDamageMarker.h"
#include "Weapons/UI/UserWidgetTest.h"
#include "Weapons/HealthComponent.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include <string>
#include "Components/WidgetComponent.h"

UHitscanWeaponComponent::UHitscanWeaponComponent() : UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
}

void UHitscanWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	reloadTime = 1.0f;
	currentAmmoClip = 30;
	currentReserves = 90;
	maxAmmo = 30;
}

void UHitscanWeaponComponent::OnFire()
{
	if(_canUse)
	{
		if(TryUseAmmo(_parent, 1.0f))
		{
			StartWaitTimer(_parent, _weaponInfo->AttackRate);
			FHitResult result;
			FVector CameraLoc;
			FRotator CameraRot;
			APlayerController* PlayerController = Cast<APlayerController>(Cast<APawn>(_parent)->GetController());
			PlayerController->GetPlayerViewPoint(CameraLoc, CameraRot);
			if(DoRaycastReturnResult(GetWorld(), result, CameraLoc, CameraLoc + CameraRot.Vector() * 10000.0f, ECollisionChannel::ECC_Visibility))
			{
				AActor* hit = result.GetActor();
				UHealthComponent* healthComponent = hit->FindComponentByClass<UHealthComponent>();
				if(healthComponent != nullptr)
				{
					healthComponent->AdjustHealth(_weaponInfo->Damage);
					UWorld* const World = GetWorld();
					FActorSpawnParameters ActorSpawnParams;
					AMyDamageMarker* object = World->SpawnActor<AMyDamageMarker>(_damageMarker, result.Location, FRotator(0.0f, 0.0f, 0.0f), ActorSpawnParams);
					if(object != nullptr)
					{
						object->SetSpawnedBy(_parent);
						object->SetText(_weaponInfo->Damage);
					}
				}
			}
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CurrentAmmoInClip:") + FString::FromInt(currentAmmoClip) + " CurrentReserves:" + FString::FromInt(currentReserves));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Attempt Reload"));
			TryReload(_parent);
		}
	}
}

void UHitscanWeaponComponent::StartReloadAmmo(AActor* actor)
{
	if(!reloading)
	{
		reloading = true;
		UWorld* world = actor->GetWorld();
		world->GetTimerManager().SetTimer(reloadTimerHandler, this, &IHasAmmo::ReloadEnded, reloadTime, false);
	}
}

void UHitscanWeaponComponent::StartWaitTimer(AActor* actor, float time)
{
	StartUse();
	UWorld* world = GetWorld();
	world->GetTimerManager().SetTimer(waitTimeHandler, this, &UHitscanWeaponComponent::EndUse, time, false);
}

void UHitscanWeaponComponent::CancelReload(AActor* actor)
{
	UWorld* world = actor->GetWorld();
	world->GetTimerManager().ClearTimer(reloadTimerHandler);
}

void UHitscanWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHitscanWeaponComponent::InitializeWeapon(UGunItem* gunItem)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "142312");
	
	Super::InitializeWeapon(gunItem);

	reloadTime = _weaponInfo->ReloadSpeed;
	currentAmmoClip = gunItem->GetAmmoInClip();
	currentReserves = gunItem->GetAmmoCount();
	maxAmmo = _weaponInfo->ClipSize;
}

void UHitscanWeaponComponent::DropWeapon()
{
	if (_weaponItem != nullptr)
	{
		_weaponItem->SetAmmoCount(currentReserves);
		_weaponItem->SetAmmoInClip(currentAmmoClip);
	}
}
