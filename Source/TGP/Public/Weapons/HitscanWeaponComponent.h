// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/WeaponInterfaces.h"
#include "Weapons/WeaponComponent.h"
#include "HitscanWeaponComponent.generated.h"

/**
 * 
 */
UCLASS()
class TGP_API UHitscanWeaponComponent : public UWeaponComponent, public IWaitTimer, public IDealDamage, public IHasAmmo, public ICanHitScan
{
	GENERATED_BODY()
public:
	UHitscanWeaponComponent();
	virtual void OnFire() override;
};
