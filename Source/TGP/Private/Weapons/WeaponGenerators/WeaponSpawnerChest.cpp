// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponGenerators/WeaponSpawnerChest.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/HealthComponent.h"
#include "TGP/TGPGameModeBase.h"
#include "Item/ItemActor.h"

// Sets default values
AWeaponSpawnerChest::AWeaponSpawnerChest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(_sceneRoot);
	
	_staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_staticMesh->SetupAttachment(_sceneRoot);

	_health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	AddOwnedComponent(_health);
}

// Called when the game starts or when spawned
void AWeaponSpawnerChest::BeginPlay()
{
	Super::BeginPlay();
	
	_health->onComponentDead.AddDynamic(this, &AWeaponSpawnerChest::OpenChest);
}

// Called every frame
void AWeaponSpawnerChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponSpawnerChest::OpenChest(AController* causer)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Open"));
	ATGPGameModeBase* GameMode = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	UGunItem* newGun = GameMode->CreateItemByUniqueId<UGunItem>(72953608, 1);
	AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(_itemActorClass, GetActorLocation(), FRotator());
	ItemActor->Initialize(newGun);
	ItemActor->AddInitialThrowForce(FVector(0.0f, 0.0f, 1.0f), 400000.0f);
}
