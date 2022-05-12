// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TGP_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY() AActor* _parent;
	UPROPERTY() USceneComponent* _holdPoint;

	UPROPERTY() class UStaticMeshComponent* _parentMesh;
	
	bool _canUse;

	UPROPERTY() class UGunItem* _weaponItem;
	UPROPERTY() class UGunInfo* _weaponInfo;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnFire();
	
	void PickupWeapon(AActor* parent);
	virtual void DropWeapon();
	void SetParentMesh(class UStaticMeshComponent* mesh) { _parentMesh = mesh; }
	virtual void InitializeWeapon(class UGunItem* gunItem);

	void StartUse(){ _canUse = false;	}
	void EndUse(){ _canUse = true;	}
};
