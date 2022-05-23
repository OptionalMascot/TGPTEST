// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interactables/IInteractable.h"
#include "FP_FirstPersonCharacter.generated.h"

class UInputComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USoundBase;
class UAnimMontage;
class UWeaponItem;
class UPlayerInventory;
class AGrenadeWeapon;
class UWeaponComponent;

UCLASS(config=Game)
class AFP_FirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** Gun mesh */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FP_Gun;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UPlayerInventory* PlayerInventory;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UWeaponComponent* WeaponComponent;

public:
	AFP_FirstPersonCharacter();

	UPlayerInventory* GetInventory() const { return PlayerInventory; }
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* CombatMontage;

	/* This is when calculating the trace to determine what the weapon has hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponRange;
	
	/* This is multiplied by the direction vector when the weapon trace hits something to apply velocity to the component that is hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	FVector WeaponAimLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	FVector WeaponDefaultLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	FRotator WeaponDefaultRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	FRotator MeshDefaultRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	FRotator AimRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	float WeaponYawDiff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	float WeaponPitchDiff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	bool IsSprinting;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	bool IsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	bool IsReloading;
	

protected:

	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Fires a virtual projectile. */
	void OnFire();

	void OnFireWeapon();

	void OnFireWeaponRelease();
	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	void ChangeWeapon(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/* 
	 * Performs a trace between two points
	 * 
	 * @param	StartTrace	Trace starting point
	 * @param	EndTrac		Trace end point
	 * @returns FHitResult returns a struct containing trace result - who/what the trace hit etc.
	 */
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Structure that handles touch data so we can process the various stages of touch. */
	struct TouchData
	{
		TouchData() { bIsPressed = false; Location = FVector::ZeroVector; }
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};

	/*
	 * Handle begin touch event.
	 * Stores the index and location of the touch in a structure
	 *
	 * @param	FingerIndex	The touch index
	 * @param	Location	Location of the touch
	 */
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/*
	 * Handle end touch event.
	 * If there was no movement processed this will fire a projectile, otherwise this will reset pressed flag in the touch structure
	 *
	 * @param	FingerIndex	The touch index
	 * @param	Location	Location of the touch
	 */
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/*
	 * Handle touch update.
	 * This will update the look position based on the change in touching position
	 *
	 * @param	FingerIndex	The touch index
	 * @param	Location	Location of the touch
	 */
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);

	// Structure to handle touch updating
	TouchData	TouchItem;
	
	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	void TryEnableTouchscreenMovement(UInputComponent* InputComponent);
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true")) class UHealthComponent* _healthComponent;
	
	bool _fireHeld;
	
	void PickupWeapon();
	void DropWeapon();
	void ReloadWeapon();
	void ThrowUtility();
	
	bool _weaponQueued;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION() void OnWeaponChanged(UWeaponItem* WeaponItem);
	
	UFUNCTION(Server, Reliable) void SrvHitScan();
	void SrvHitScan_Implementation();
	
	UFUNCTION(Server, Reliable) void SrvShootGun();
	void SrvShootGun_Implementation();
	
	UFUNCTION(Server, Reliable) void OnChangeSelectedWeapon(int Slot);
	void OnChangeSelectedWeapon_Implementation(int Slot);

	UFUNCTION(Server, Reliable) void OnPickUpItem(class AItemActor* ItemActor, int Slot);
	void OnPickUpItem_Implementation(AItemActor* ItemActor, int Slot);
	
	UFUNCTION(Server, Reliable) void OnWeaponDropped();
	void OnWeaponDropped_Implementation();
	
	UFUNCTION(NetMulticast, Reliable) void ChangeWeaponMeshMulti(int ItemId);
	void ChangeWeaponMeshMulti_Implementation(int ItemId);

	UFUNCTION(Server, Reliable) void RequestWeaponMeshChange(int Slot);
	void RequestWeaponMeshChange_Implementation(int Slot);
	
	UPROPERTY() AActor* _lastLooked;
	IIInteractable* _lastLookedInterface;
	void InteractWithObject();
	
	void CastForInteractable(float DeltaTime);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) TSubclassOf<AGrenadeWeapon> _grenadeToSpawn;

	UFUNCTION() void OnOverlapWithActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Camera", meta=(AllowPrivateAccess = true))
	float M_DefaultCameraSensitivity;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Camera", meta=(AllowPrivateAccess = true))
	float M_CameraSensitivity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Camera", meta=(AllowPrivateAccess = true))
	float M_AimSensitivity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Camera", meta=(AllowPrivateAccess = true))
	float M_SniperSensitivity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess = true))
	float M_DefaultSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess = true))
	float M_SprintSpeed;

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintImplementableEvent) void TestDebug();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UWeaponComponent* GetCurrentWeaponComponent() { return _currentWeaponComponent; }

	void LookUp(float inputValue);
	void Turn(float inputValue);

	UFUNCTION(BlueprintImplementableEvent)
	void SetAnimation();

	UFUNCTION(BlueprintCallable)
	void SetWeaponTransformDefaults();
	
	void AttachWeapon();

	void Sprint();
	void StopSprint();

	UFUNCTION(BlueprintImplementableEvent)
	void Aim();

	UFUNCTION(BlueprintImplementableEvent)
	void StopAim();

	void BeginAim();
	UFUNCTION(BlueprintCallable)
	void EndAim();

	UFUNCTION(BlueprintCallable)
	void SwitchWeapon();

	UFUNCTION(BlueprintCallable)
	void Reload();
};

