// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Throwables/GrenadeWeapon.h"
#include "NiagaraFunctionLibrary.h"
#include "Item/ItemInfo.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AGrenadeWeapon::AGrenadeWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ItemSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemSkeletalMesh"));
	ItemSkeletalMesh->SetupAttachment(RootComponent);
	ItemSkeletalMesh->SetSimulatePhysics(true);

	_particleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Particles"));
	_particleSystem->SetupAttachment(ItemSkeletalMesh);
}

// Called when the game starts or when spawned
void AGrenadeWeapon::BeginPlay()
{
	Super::BeginPlay();
	_startFuse = false;
	SetPhysicsMesh(ItemSkeletalMesh);
	if(_particleSystem)
	{
		_particleSystem->Deactivate();
	}
}

void AGrenadeWeapon::StartWaitTimer(AActor* actor, float time)
{
	_startFuse = true;
	GetWorld()->GetTimerManager().SetTimer(waitTimeHandler, this, &AGrenadeWeapon::ExplodeGrenade, time, false);
}

void AGrenadeWeapon::ExplodeGrenade()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("BOOM"));
	_particleSystem->Activate();
	ItemSkeletalMesh->SetHiddenInGame(true);

	SphereCastForTargets();

	FTimerHandle explosionTimeHandler;
	GetWorld()->GetTimerManager().SetTimer(explosionTimeHandler, this, &AGrenadeWeapon::DestroyObj, 1.0f, false);
}

void AGrenadeWeapon::SphereCastForTargets()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	TArray<FHitResult> OutHits;
	TArray<AActor*> toIgnore { this };
	TArray<AActor*> appliedPhysics;
	if(UKismetSystemLibrary::SphereTraceMulti(GetWorld(), ItemSkeletalMesh->GetComponentLocation(), ItemSkeletalMesh->GetComponentLocation(), _throwableInfo->ExplosionRadius, ETraceTypeQuery::TraceTypeQuery1, false, toIgnore, EDrawDebugTrace::None, OutHits, true))
	{
		for(int i = 0; i < OutHits.Num(); i++)
		{
			UGameplayStatics::ApplyDamage(OutHits[i].GetActor(), _throwableInfo->Damage, _spawnedBy, _spawnedBy->GetPawn(), UDamageType::StaticClass());
			if(OutHits[i].GetActor() != nullptr)
			{
				if(!appliedPhysics.Contains(OutHits[i].GetActor()))
				{
					appliedPhysics.Add(OutHits[i].GetActor());
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, OutHits[i].GetActor()->GetName());
					UStaticMeshComponent* mesh = OutHits[i].GetActor()->FindComponentByClass<UStaticMeshComponent>();
					USkeletalMeshComponent* skeleMesh = OutHits[i].GetActor()->FindComponentByClass<USkeletalMeshComponent>();
					if(skeleMesh)
					{
						skeleMesh->AddRadialForce(ItemSkeletalMesh->GetComponentLocation(), _throwableInfo->ExplosionRadius, _throwableInfo->KnockbackForce, ERadialImpulseFalloff::RIF_Linear, false);
						continue;
					}
					if(mesh)
					{
						mesh->AddRadialForce(ItemSkeletalMesh->GetComponentLocation(), _throwableInfo->ExplosionRadius, _throwableInfo->KnockbackForce, ERadialImpulseFalloff::RIF_Linear, false);
						continue;
					}
				}
			}
		}
	}
}

void AGrenadeWeapon::DestroyObj()
{
	Destroy();
}

// Called every frame
void AGrenadeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AGrenadeWeapon::SetProjectileParameters(APlayerController* spawnedBy, FVector dir, float speed)
{
	Super::SetProjectileParameters(spawnedBy, dir, speed);
	_physicsMeshReference->AddForce(dir * speed);
	FVector randSpin = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f));
	_physicsMeshReference->AddTorque(randSpin * 50000.0f);
}

void AGrenadeWeapon::Initialize(UThrowableInfo* throwableInfo)
{
	_throwableInfo = throwableInfo;
	ItemSkeletalMesh->SetSkeletalMesh(_throwableInfo->WeaponSkeletalMesh);
	StartWaitTimer(this, _throwableInfo->FuseTime);	
}

