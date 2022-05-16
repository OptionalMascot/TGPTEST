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
	singleFireRecoilStarted = false;
}

void UHitscanWeaponComponent::OnFire()
{
	if(!CheckMouseReleased())
		return;
	
	if(_canUse)
	{
		int infiniteCheck = _weaponInfo->UnlimitedAmmo ? 0.0f : 1.0f;
		if(TryUseAmmo(_parent, infiniteCheck))
		{
			FHitResult result;
			FVector CameraLoc;
			FRotator CameraRot;
			if(_parentController == nullptr)
			{
				_parentController = Cast<APlayerController>(Cast<APawn>(_parent)->GetController());
			}
			_parentController->GetPlayerViewPoint(CameraLoc, CameraRot);


			// Recoil Funcs
			if(!HasStartedRecoil())
			{
				StartRecoil(CameraRot);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OriginRotator: ") + CameraRot.ToString());
			}
			
			StartTimeline();
			if(_weaponInfo->FireType == EFireType::Single)
			{
				ApplySingleFire();
			}
			
			StartWaitTimer(_parent, _weaponInfo->AttackRate);
			
			DrawDebugLine(GetWorld(), _parentMesh->GetComponentTransform().GetLocation() + FVector(0.0f, 0.0f, 15.0f), CameraLoc + CameraRot.Vector() * 10000.0f, FColor::Red, false, 5.0f, 0, 1.0f);


			if(DoRaycastReturnResult(GetWorld(), result, CameraLoc, CameraLoc + CameraRot.Vector() * 10000.0f, ECollisionChannel::ECC_Visibility))
			{
				AActor* hit = result.GetActor();
				float dealtDamage = UGameplayStatics::ApplyDamage(hit, _weaponInfo->Damage, _parentController, GetOwner(), UDamageType::StaticClass());
			}
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CurrentAmmoInClip:") + FString::FromInt(currentAmmoClip) + " CurrentReserves:" + FString::FromInt(currentReserves));
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Attempt Reload"));
			TryReload(_parent);
		}
	}
}

void UHitscanWeaponComponent::OnFireEnd()
{
	if(_weaponInfo->FireType != EFireType::Single)
	{
		ReverseTimeline(_weaponInfo->RecoilRecoveryModifier);
	}
	_singleFireCheck = false;
	
	FVector CameraLoc;
	FRotator CameraRot;
	if(_parentController == nullptr)
	{
		_parentController = Cast<APlayerController>(Cast<APawn>(_parent)->GetController());
	}
	_parentController->GetPlayerViewPoint(CameraLoc, CameraRot);
	postRecoilRotation = CameraRot;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PostRecoilRotation: ") + postRecoilRotation.ToString());
}

void UHitscanWeaponComponent::StartReloadAmmo(AActor* actor)
{
	if(!reloading)
	{
		reloading = true;
		GetWorld()->GetTimerManager().SetTimer(reloadTimerHandler, this, &IHasAmmo::ReloadEnded, reloadTime, false);
	}
}

void UHitscanWeaponComponent::StartWaitTimer(AActor* actor, float time)
{
	StartUse();
	GetWorld()->GetTimerManager().SetTimer(waitTimeHandler, this, &UHitscanWeaponComponent::EndUse, time, false);
}

void UHitscanWeaponComponent::CancelReload(AActor* actor)
{
	GetWorld()->GetTimerManager().ClearTimer(reloadTimerHandler);
}

void UHitscanWeaponComponent::RecoilTimelineProgressPitch(float Value)
{
	if(GetTimelineDirection() == ERecoilDirection::Forwards)
		Value *= -1;

	ApplyRecoilPitch(_parentController, Value * recoilTimeline.GetPlayRate());
}

void UHitscanWeaponComponent::RecoilTimelineProgressYaw(float Value)
{
	if(GetTimelineDirection() == ERecoilDirection::Forwards)
		Value *= -1;

	ApplyRecoilYaw(_parentController, Value * recoilTimeline.GetPlayRate());
}

void UHitscanWeaponComponent::RecoilTimelineFinished()
{
	if(_weaponInfo->FireType == EFireType::Single)
		SingleFireRecoilReset();

	if(GetTimelineDirection() == ERecoilDirection::Backwards)
	{
		EndRecoil();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("BACKWARDS FINISHED"));
	}
}

void UHitscanWeaponComponent::SingleFireRecoilReset()
{
	if(!recoilTimeline.IsReversing() && singleFireRecoilStarted)
	{
		ReverseTimeline(_weaponInfo->RecoilRecoveryModifier);
		singleFireRecoilStarted = false;
	}
}

void UHitscanWeaponComponent::ResetRecoilTimeline()
{
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
		TimelineProgressSingleFire.BindUFunction(this, FName("RecoilTimelineFinished"));
		recoilTimeline.SetTimelineFinishedFunc(TimelineProgressSingleFire);
	}
}


void UHitscanWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	recoilTimeline.TickTimeline(DeltaTime);

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OriginRotator: ") + originRotation.ToString());
	//
}

void UHitscanWeaponComponent::InitializeWeapon(UGunItem* gunItem)
{	
	Super::InitializeWeapon(gunItem);

	reloadTime = _weaponInfo->ReloadSpeed;
	currentAmmoClip = gunItem->GetAmmoInClip();
	currentReserves = gunItem->GetAmmoCount();
	maxAmmo = _weaponInfo->ClipSize;
	
	ResetRecoilTimeline();
}

void UHitscanWeaponComponent::DropWeapon()
{
	if (_weaponItem != nullptr)
	{
		_weaponItem->SetAmmoCount(currentReserves);
		_weaponItem->SetAmmoInClip(currentAmmoClip);
	}
}
