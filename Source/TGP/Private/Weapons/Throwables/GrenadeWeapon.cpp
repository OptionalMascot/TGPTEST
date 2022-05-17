// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Throwables/GrenadeWeapon.h"

#include "Item/ItemInfo.h"

// Sets default values
AGrenadeWeapon::AGrenadeWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ItemSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemSkeletalMesh"));
	ItemSkeletalMesh->SetupAttachment(RootComponent);
	ItemSkeletalMesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void AGrenadeWeapon::BeginPlay()
{
	Super::BeginPlay();
	_startFuse = false;
	Initialize(_throwableInfo);
}

void AGrenadeWeapon::StartWaitTimer(AActor* actor, float time)
{
	_startFuse = true;
	GetWorld()->GetTimerManager().SetTimer(waitTimeHandler, this, &AGrenadeWeapon::ExplodeGrenade, time, false);
}

void AGrenadeWeapon::ExplodeGrenade()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("BOOM"));
	Destroy();
}

// Called every frame
void AGrenadeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AGrenadeWeapon::SetInitialThrowForce(FVector forceDir)
{
	ItemSkeletalMesh->AddForce(forceDir);
	FVector randSpin = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f));
	ItemSkeletalMesh->AddTorque(randSpin * 50000.0f);
}

void AGrenadeWeapon::Initialize(UThrowableInfo* throwableInfo)
{
	_throwableInfo = throwableInfo;
	ItemSkeletalMesh->SetSkeletalMesh(_throwableInfo->WeaponSkeletalMesh);
	StartWaitTimer(this, _throwableInfo->FuseTime);	
}

