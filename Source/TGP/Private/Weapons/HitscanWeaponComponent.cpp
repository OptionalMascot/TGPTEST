// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/HitscanWeaponComponent.h"
#include "Weapons/UI/MyDamageMarker.h"
#include "Weapons/UI/UserWidgetTest.h"
#include "Weapons/HealthComponent.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TGP/FP_FirstPerson/FP_FirstPersonCharacter.h"

UHitscanWeaponComponent::UHitscanWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}

void UHitscanWeaponComponent::SrvOnFire_Implementation()
{
	Super::SrvOnFire_Implementation();

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "THISIDJIJWAD");

	FHitResult result;
	FVector CameraLoc;
	FRotator CameraRot;

	if(_parentController == nullptr)
		_parentController = Cast<APlayerController>(Cast<APawn>(_parent)->GetController());
	
	_parentController->GetPlayerViewPoint(CameraLoc, CameraRot);

	DrawDebugLine(GetWorld(), _parentMesh->GetComponentLocation() + FVector(0.0f, 0.0f, 15.0f), CameraLoc + CameraRot.Vector() * 10000.0f, FColor::Red, false, 1.0f, 0, 1.0f);

	if(DoRaycastReturnResult(GetWorld(), result, _parentMesh->GetComponentLocation(), CameraLoc + CameraRot.Vector() * 10000.0f, ECollisionChannel::ECC_Visibility)) // If hitting something
	{
		AActor* hit = result.GetActor(); // Get Actor
		UGameplayStatics::ApplyDamage(hit, 100.f, nullptr, nullptr, UDamageType::StaticClass());
		
		//float dealtDamage = UGameplayStatics::ApplyDamage(hit, _weaponInfo->Damage, _parentController, GetOwner(), UDamageType::StaticClass()); // Attempt to apply damage
	}
}

void UHitscanWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	reloadTime = 1.0f;
	currentAmmoClip = 30;
	currentReserves = 90;
	maxAmmo = 30;
	singleFireRecoilStarted = false;
	recoilMaxThreshhold = 1.0f;
	notPlayedFullyValue = 1.0f;
}

void UHitscanWeaponComponent::OnFire()
{
	if(!CheckMouseReleased()) // If single fire check is turned on, require a release before firing again
		return;
	
	if(_canUse)
	{
		int infiniteCheck = _weaponInfo->UnlimitedAmmo ? 0.0f : 1.0f; // Only decrease ammo if not set to unlimited
		if(TryUseAmmo(GetOwner(), infiniteCheck))
		{
			// Get the current look rotation for recoil purposes
			FHitResult result;
			FVector CameraLoc;
			FRotator CameraRot;
			
			if(_parentController == nullptr)
			{
				const APawn* Pawn = Cast<APawn>(GetOwner());
				_parentController = Cast<APlayerController>(Pawn->GetController());
			}
			
			_parentController->GetPlayerViewPoint(CameraLoc, CameraRot);
			
			// Recoil Funcs
			if(!HasStartedRecoil()) // If recoil hasn't already been started
			{
				StartRecoil(CameraRot); // Start recoil storing the current rotation
				StartTimeline(); // Begin the recoil timeline
			}

			if(_weaponInfo->FireType == EFireType::Single) // If single fire is turned on
			{
				ApplySingleFire(); // Reset the timeline so recoil begins from the start, and enable the check
			}
			
			StartWaitTimer(_parent, _weaponInfo->AttackRate); // Start timer for gun to be able to shoot again
			
			//DrawDebugLine(GetWorld(), _parentMesh->GetComponentTransform().GetLocation() + FVector(0.0f, 0.0f, 15.0f), CameraLoc + CameraRot.Vector() * 10000.0f, FColor::Red, false, 0.0f, 0, 1.0f);

			SrvOnFire();
			
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CurrentAmmoInClip:") + FString::FromInt(currentAmmoClip) + " CurrentReserves:" + FString::FromInt(currentReserves));
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Attempt Reload"));
			TryReload(); // If can't shoot, try and reload
		}
	}
}

void UHitscanWeaponComponent::OnFireEnd() // Called by parent on releasing left click
{
	if(_weaponInfo->FireType != EFireType::Single)
	{
		// recoilTimeline.SetNewTime(recoilTimeline.GetTimelineLength()); Make a new modifier to scale the value by how much of the timeline has played
		notPlayedFullyValue = recoilTimeline.GetTimelineLength() / recoilTimeline.GetPlaybackPosition();
		ReverseTimeline(_weaponInfo->RecoilRecoveryModifier); // Reverse the timeline
		FVector CameraLoc;
		FRotator CameraRot;
		if(_parentController == nullptr)
		{
			_parentController = Cast<APlayerController>(Cast<APawn>(_parent)->GetController());
		}
		_parentController->GetPlayerViewPoint(CameraLoc, CameraRot);
		postRecoilRotation = CameraRot; // Set post recoil rotation for recoil overriding
	}
	_singleFireCheck = false; // Allow single fire to shoot again

	// The only reason the above doesnt happen for single fire, is that the timeline should play in full for single fire
}

void UHitscanWeaponComponent::StartReloadAmmo()
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

// Following two functions are callbacks for timeline
// If playing forward, the curve result is flipped to allow pitch to move upwards
// If backwards and full auto, AdjustRecoilForCompensate judges how far the gun should move back down, and cancels any movement lower than origin
void UHitscanWeaponComponent::RecoilTimelineProgressPitch(float Value)
{
	ApplyRecoilPitch(_parentController, Value * 100.0f, _weaponInfo->FireType == EFireType::Single);
}

void UHitscanWeaponComponent::RecoilTimelineProgressYaw(float Value)
{
	ApplyRecoilYaw(_parentController, Value * 100.0f, _weaponInfo->FireType == EFireType::Single);
}

void UHitscanWeaponComponent::RecoilTimelineFinished()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Timeline Ended"));
	if(GetTimelineDirection() == ERecoilDirection::Backwards && !singleFireRecoilStarted)
	{
		EndRecoil();
	}
	
	if(_weaponInfo->FireType == EFireType::Single)
		SingleFireRecoilReset();
}

void UHitscanWeaponComponent::SingleFireRecoilReset()
{
	if(!recoilTimeline.IsReversing() && singleFireRecoilStarted) // Once recoil has finished for single fire, play it backwards as expected
	{
		ReverseTimeline(_weaponInfo->RecoilRecoveryModifier);
		singleFireRecoilStarted = false;
	}
}

void UHitscanWeaponComponent::ResetRecoilTimeline() // Resets the recoil timeline variables once a new gun has been picked up, passing in new curves
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
}

void UHitscanWeaponComponent::InitializeWeapon(UGunItem* gunItem)
{	
	Super::InitializeWeapon(gunItem);

	reloadTime = _weaponInfo->ReloadSpeed;
	currentAmmoClip = gunItem->GetAmmoInClip();
	currentReserves = gunItem->GetAmmoCount();
	maxAmmo = _weaponInfo->ClipSize;
	
	ResetRecoilTimeline();
	notPlayedFullyValue = 1.0f;
	EndRecoil();
	startedRecoil = false;
	singleFireRecoilStarted = false;
	recoilTimelineDirection = ERecoilDirection::Forwards;
}

void UHitscanWeaponComponent::DropWeapon()
{
	if (_weaponItem != nullptr)
	{
		_weaponItem->SetAmmoCount(currentReserves);
		_weaponItem->SetAmmoInClip(currentAmmoClip);
	}
}
