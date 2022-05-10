// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponComponent.h"
#include "Components/SceneComponent.h"
#include "Item/ItemInfo.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
	_canUse = true;
	_weaponInfo = nullptr;
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UWeaponComponent::OnFire()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Pew Pew"));
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
		_parentMesh->SetSimulatePhysics(false);
		_parentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UWeaponComponent::DropWeapon()
{
	_parent = nullptr;
	_parentMesh->SetSimulatePhysics(false);
	_parentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UWeaponComponent::InitializeWeapon(UGunInfo* info)
{
	_weaponInfo = info;
}

