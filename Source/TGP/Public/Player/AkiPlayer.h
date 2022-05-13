// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AkiPlayer.generated.h"

UCLASS()
class TGP_API AAkiPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAkiPlayer();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Player")
	class USkeletalMeshComponent* PlayerMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon")
	class USceneComponent* AimLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Camera")
	class UCameraComponent* FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh Default Transform")
	FVector DefaultMeshLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Player Movement")
	bool IsAiming;

	bool IsSprinting;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Camera Settings", meta=(AllowPrivateAccess = true))
	float M_CameraSensitivity; //Try and keep between 0-1 otherwise sensitivity becomes too high

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Player Movement", meta=(AllowPrivateAccess = true))
	float M_SprintSpeed = 1700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Player Movement", meta=(AllowPrivateAccess = true))
	float M_DefaultSpeed = 600.0f;

	float DefaultFieldOfView;
	
	bool M_MovingForward;
	bool M_MovingRight;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float inputValue);
	void MoveRight(float inputValue);
	
	void LookAround(float inputValue);
	void LookUp(float inputValue);

	void Sprint();
	void StopSprinting();

	void ADS();
	void StopADS();

	UFUNCTION(BlueprintImplementableEvent)
	void BeginAimTransisiton();

	UFUNCTION(BlueprintImplementableEvent)
	void BeginUnAimTransisiton();

	void AdjustFOV(float ZoomLevel);
};
