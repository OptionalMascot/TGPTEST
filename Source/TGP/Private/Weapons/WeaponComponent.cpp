// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponComponent.h"
#include "Components/SceneComponent.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
	_canUse = true;
	_singleFireCheck = false;
	_weaponInfo = nullptr;
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UWeaponComponent::CheckMouseReleased()
{
	if(_weaponInfo->FireType == EFireType::Single)
	{
		if(!_singleFireCheck)
		{
			_singleFireCheck = true;
			return true;
		}
		return false;
	}
	return true;
}


void UWeaponComponent::OnFire()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Pew Pew"));
}

void UWeaponComponent::OnFireEnd()
{

}

// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::PickupWeapon(AActor* parent)
{
	if(_parent == nullptr)
	{
		_parent = parent;
		_parentController = nullptr;
	}
}

void UWeaponComponent::DropWeapon()
{
	
}

void UWeaponComponent::InitializeWeapon(UGunItem* gunItem) // Get and Update ammo count from _weaponItem
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "grfeasdfewa");
	
	DropWeapon();
	
	_weaponItem = gunItem;
	_weaponInfo = Cast<UGunInfo>(gunItem->GetItemInfo());
}

