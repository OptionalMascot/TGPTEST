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

	bool CheckMouseReleased();
	
	UPROPERTY() AActor* _parent;
	UPROPERTY() APlayerController* _parentController;
	UPROPERTY() USceneComponent* _holdPoint;

	UPROPERTY() class USkeletalMeshComponent* _parentMesh;
	
	bool _canUse;
	bool _singleFireCheck;
	
	UPROPERTY() class UGunItem* _weaponItem;
	UPROPERTY() class UGunInfo* _weaponInfo;
	
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnFire();
	virtual void OnFireEnd();
	
	void PickupWeapon(AActor* parent);
	virtual void DropWeapon();
	void SetParentMesh(class USkeletalMeshComponent* mesh) { _parentMesh = mesh; }
	virtual void InitializeWeapon(class UGunItem* gunItem);
	UFUNCTION(BlueprintCallable) UGunInfo* GetWeaponInfo() { return _weaponInfo; }
	
	void StartUse(){ _canUse = false;	}
	void EndUse(){ _canUse = true;	}
};
