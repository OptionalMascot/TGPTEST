// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitscanWeaponComponent.h"

#include <mftransform.h>

#include "Weapons/UI/MyDamageMarker.h"
#include "Weapons/UI/UserWidgetTest.h"
#include "Weapons/HealthComponent.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include <string>
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

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
	_singleFireRecoilStarted = false;
}

void UHitscanWeaponComponent::OnFire()
{
	if(!CheckMouseReleased())
		return;
	
	if(_canUse)
	{
		if(TryUseAmmo(_parent, 0.0f))
		{
			recoilTimeline.Play();
			recoilTimeline.SetPlayRate(1.0f);
			recoilTimelineForward = true;
			if(_weaponInfo->FireType == EFireType::Single)
			{
				_singleFireRecoilStarted = true;
				recoilTimeline.SetNewTime(0);
			}
			
			StartWaitTimer(_parent, _weaponInfo->AttackRate);
			FHitResult result;
			FVector CameraLoc;
			FRotator CameraRot;
			if(_parentController == nullptr)
			{
				_parentController = Cast<APlayerController>(Cast<APawn>(_parent)->GetController());
			}
			_parentController->GetPlayerViewPoint(CameraLoc, CameraRot);

			
			DrawDebugLine(GetWorld(), _parentMesh->GetComponentTransform().GetLocation() + FVector(0.0f, 0.0f, 15.0f), CameraLoc + CameraRot.Vector() * 10000.0f, FColor::Red, false, 5.0f, 0, 1.0f);


			if(DoRaycastReturnResult(GetWorld(), result, CameraLoc, CameraLoc + CameraRot.Vector() * 10000.0f, ECollisionChannel::ECC_Visibility))
			{
				AActor* hit = result.GetActor();
				float dealtDamage = UGameplayStatics::ApplyDamage(hit, _weaponInfo->Damage, _parentController, GetOwner(), UDamageType::StaticClass());
				if(dealtDamage != 0.0f)
				{
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

void UHitscanWeaponComponent::OnFireEnd()
{
	if(_weaponInfo->FireType != EFireType::Single)
	{
		recoilTimeline.Reverse();
		recoilTimelineForward = false;
		recoilTimeline.SetPlayRate(_weaponInfo->RecoilRecoveryModifier);
	}
	_singleFireCheck = false;
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

void UHitscanWeaponComponent::RecoilTimelineProgressPitch(float Value)
{
	if(recoilTimelineForward)
		Value *= -1;

	ApplyRecoilPitch(_parentController, Value * recoilTimeline.GetPlayRate());
}

void UHitscanWeaponComponent::RecoilTimelineProgressYaw(float Value)
{
	if(recoilTimelineForward)
		Value *= -1;

	ApplyRecoilYaw(_parentController, Value * recoilTimeline.GetPlayRate());
}

void UHitscanWeaponComponent::SingleFireRecoilReset()
{
	if(_weaponInfo->FireType == EFireType::Single && !recoilTimeline.IsReversing() && _singleFireRecoilStarted)
	{
		recoilTimeline.Reverse();
		recoilTimelineForward = false;
		recoilTimeline.SetPlayRate(_weaponInfo->RecoilRecoveryModifier);
		_singleFireRecoilStarted = false;
	}
}


void UHitscanWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	recoilTimeline.TickTimeline(DeltaTime);
}

void UHitscanWeaponComponent::InitializeWeapon(UGunItem* gunItem)
{	
	Super::InitializeWeapon(gunItem);

	reloadTime = _weaponInfo->ReloadSpeed;
	currentAmmoClip = gunItem->GetAmmoInClip();
	currentReserves = gunItem->GetAmmoCount();
	maxAmmo = _weaponInfo->ClipSize;
	
	recoilTimeline = FTimeline();
	if(_weaponInfo->RecoilCurvePitch && _weaponInfo->RecoilCurveYaw)
	{
		FOnTimelineFloat TimelineProgressPitch;
		TimelineProgressPitch.BindUFunction(this, FName("RecoilTimelineProgressPitch"));
		FOnTimelineFloat TimelineProgressYaw;
		TimelineProgressYaw.BindUFunction(this, FName("RecoilTimelineProgressYaw"));
	    recoilTimeline.AddInterpFloat(_weaponInfo->RecoilCurvePitch, TimelineProgressPitch);
		recoilTimeline.AddInterpFloat(_weaponInfo->RecoilCurveYaw, TimelineProgressYaw);
		FOnTimelineEvent TimelineProgressSingleFire;
		TimelineProgressSingleFire.BindUFunction(this, FName("SingleFireRecoilReset"));
		recoilTimeline.SetTimelineFinishedFunc(TimelineProgressSingleFire);
	}
}

void UHitscanWeaponComponent::DropWeapon()
{
	if (_weaponItem != nullptr)
	{
		_weaponItem->SetAmmoCount(currentReserves);
		_weaponItem->SetAmmoInClip(currentAmmoClip);
	}
}
