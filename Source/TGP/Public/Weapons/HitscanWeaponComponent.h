// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/WeaponInterfaces.h"
#include "Weapons/WeaponComponent.h"
#include "HitscanWeaponComponent.generated.h"

UCLASS()
class TGP_API UHitscanWeaponComponent : public UWeaponComponent, public IWaitTimer, public IHasAmmo, public ICanHitScan, public IUseRecoil
{
	GENERATED_BODY()
protected:
	virtual void StartReloadAmmo(AActor* actor) override;
	virtual void StartWaitTimer(AActor* actor, float time) override;
	virtual void CancelReload(AActor* actor) override;

	UFUNCTION() void RecoilTimelineProgressPitch(float Value);
	UFUNCTION() void RecoilTimelineProgressYaw(float Value);
	UFUNCTION() void SingleFireRecoilReset();

	void ResetRecoilTimeline();
	
    UPROPERTY(EditAnywhere) TSubclassOf<class AMyDamageMarker> _damageMarker;

	FTimeline recoilTimeline;
	UPROPERTY() class UCurveFloat* _curve;
	bool recoilTimelineForward;
	bool _singleFireRecoilStarted;
public:
	UHitscanWeaponComponent();
	virtual void BeginPlay() override;
	virtual void OnFire() override;
	virtual void OnFireEnd() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeWeapon(UGunItem* gunItem) override;
	virtual void DropWeapon() override;
};
