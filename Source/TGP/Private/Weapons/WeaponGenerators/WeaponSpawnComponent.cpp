// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponGenerators/WeaponSpawnComponent.h"

#include <ThirdParty/asio/1.12.2/asio/impl/read.hpp>

#include "Item/ItemActor.h"
#include "Item/ItemInfo.h"
#include "Kismet/GameplayStatics.h"
#include "TGP/TGPGameModeBase.h"

// Sets default values for this component's properties
UWeaponSpawnComponent::UWeaponSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...

	_itemSpawnMode = EItemSpawn::Random;
}


// Called when the game starts
void UWeaponSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UWeaponSpawnComponent::SpawnWeapon()
{
	switch(_itemSpawnMode)
	{
		case EItemSpawn::Random:
			SpawnRandomWeapon();
			break;
		case EItemSpawn::Set:
			SpawnSetWeapon();
			break;
		case EItemSpawn::RandomRarity:
			break;
		default:
			SpawnRandomWeapon();
			break;
	}
}

void UWeaponSpawnComponent::SpawnRandomWeapon()
{
	ATGPGameModeBase* GameMode = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	UGunItem* newGun = GameMode->CreateItemByUniqueId<UGunItem>(72953608, 1);
	AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(_itemActorClass, GetOwner()->GetActorLocation(), FRotator());
	ItemActor->Initialize(newGun);
	ItemActor->AddInitialThrowForce(FVector(0.0f, 0.0f, 1.0f), 400000.0f);
}

void UWeaponSpawnComponent::SpawnRandomRarityWeapon()
{
	
}

void UWeaponSpawnComponent::SpawnSetWeapon()
{
	if(_gunInfo)
	{
		ATGPGameModeBase* GameMode = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		UGunItem* newGun = GameMode->CreateItemByUniqueId<UGunItem>(_gunInfo->UniqueId, 1);
		AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(_itemActorClass, GetOwner()->GetActorLocation(), FRotator());
		ItemActor->Initialize(newGun);
		ItemActor->AddInitialThrowForce(FVector(0.0f, 0.0f, 1.0f), 400000.0f);
	}
}



