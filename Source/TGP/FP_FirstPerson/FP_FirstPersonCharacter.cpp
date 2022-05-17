#include "FP_FirstPersonCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/GunHostActor.h"
#include "Weapons/WeaponComponent.h"
#include "Weapons/Interfaces/WeaponInterfaces.h"
#include "Inventory/PlayerInventory.h"
#include "Item/BaseItem.h"
#include "Item/ItemActor.h"
#include "Item/ItemInfo.h"
#include "Weapons/Throwables/GrenadeWeapon.h"

#define COLLISION_WEAPON		ECC_GameTraceChannel1

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

AFP_FirstPersonCharacter::AFP_FirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0, 0, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);				// Set so only owner can see mesh
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);	// Attach mesh to FirstPersonCameraComponent
	Mesh1P->bCastDynamicShadow = false;			// Disallow mesh to cast dynamic shadows
	Mesh1P->CastShadow = false;				// Disallow mesh to cast other shadows

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// Only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;		// Disallow mesh to cast dynamic shadows
	FP_Gun->CastShadow = false;			// Disallow mesh to cast other shadows
	FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));

	GunActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("GunActor"));
	GunActorComponent->SetChildActorClass(AGunHostActor::StaticClass());
	GunActorComponent->SetupAttachment(Mesh1P);

	PlayerInventory = CreateDefaultSubobject<UPlayerInventory>(TEXT("PlayerInventory"));
	AddOwnedComponent(PlayerInventory);

	// Set weapon damage and range
	WeaponRange = 5000.0f;
	WeaponDamage = 500000.0f;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFP_FirstPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	
	// Set up gameplay key bindings

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFP_FirstPersonCharacter::OnFireWeapon);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFP_FirstPersonCharacter::OnFireWeaponRelease);
	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AFP_FirstPersonCharacter::InteractWithObject);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AFP_FirstPersonCharacter::DropWeapon);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFP_FirstPersonCharacter::ReloadWeapon);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AFP_FirstPersonCharacter::ThrowUtility);
	
	// Attempt to enable touch screen movement
	TryEnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFP_FirstPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFP_FirstPersonCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFP_FirstPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFP_FirstPersonCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("ChangeWeapon", this, &AFP_FirstPersonCharacter::ChangeWeapon);
}

void AFP_FirstPersonCharacter::ChangeWeapon(float Val)
{
	if (Val != 0.f)
		PlayerInventory->ChangeWeapon(EWeaponSlot(Val - 1.f));
}

void AFP_FirstPersonCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
}

void AFP_FirstPersonCharacter::OnFire()
{
	// Play a sound if there is one
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	// Now send a trace from the end of our gun to see if we should hit anything
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	
	FVector ShootDir = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;

	if (PlayerController)
	{
		// Calculate the direction of fire and the start location for trace
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(StartTrace, CamRot);
		ShootDir = CamRot.Vector();

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		StartTrace = StartTrace + ShootDir * ((GetActorLocation() - StartTrace) | ShootDir);
	}

	// Calculate endpoint of trace
	const FVector EndTrace = StartTrace + ShootDir * WeaponRange;

	// Check for impact
	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	// Deal with impact
	AActor* DamagedActor = Impact.GetActor();
	UPrimitiveComponent* DamagedComponent = Impact.GetComponent();

	// If we hit an actor, with a component that is simulating physics, apply an impulse
	if ((DamagedActor != nullptr) && (DamagedActor != this) && (DamagedComponent != nullptr) && DamagedComponent->IsSimulatingPhysics())
	{
		//DamagedComponent->AddImpulseAtLocation(ShootDir * WeaponDamage, Impact.Location);
	}


	if(_currentWeaponComponent != nullptr)
	{
		_currentWeaponComponent->OnFire();
	}
}

void AFP_FirstPersonCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// If touch is already pressed check the index. If it is not the same as the current touch assume a second touch and thus we want to fire
	if (TouchItem.bIsPressed == true)
	{
		if( TouchItem.FingerIndex != FingerIndex)
		{
			OnFire();			
		}
	}
	else 
	{
		// Cache the finger index and touch location and flag we are processing a touch
		TouchItem.bIsPressed = true;
		TouchItem.FingerIndex = FingerIndex;
		TouchItem.Location = Location;
		TouchItem.bMoved = false;
	}
}

void AFP_FirstPersonCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// If we didn't record the start event do nothing, or this is a different index
	if((TouchItem.bIsPressed == false) || ( TouchItem.FingerIndex != FingerIndex) )
	{
		return;
	}

	// If the index matches the start index and we didn't process any movement we assume we want to fire
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}

	// Flag we are no longer processing the touch event
	TouchItem.bIsPressed = false;
}

void AFP_FirstPersonCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// If we are processing a touch event and this index matches the initial touch event process movement
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (GetWorld() != nullptr)
		{
			UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
			if (ViewportClient != nullptr)
			{
				FVector MoveDelta = Location - TouchItem.Location;
				FVector2D ScreenSize;
				ViewportClient->GetViewportSize(ScreenSize);
				FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
				if (FMath::Abs(ScaledDelta.X) >= (4.0f / ScreenSize.X))
				{
					TouchItem.bMoved = true;
					float Value = ScaledDelta.X * BaseTurnRate;
					AddControllerYawInput(Value);
				}
				if (FMath::Abs(ScaledDelta.Y) >= (4.0f / ScreenSize.Y))
				{
					TouchItem.bMoved = true;
					float Value = ScaledDelta.Y* BaseTurnRate;
					AddControllerPitchInput(Value);
				}
				TouchItem.Location = Location;
			}
			TouchItem.Location = Location;
		}
	}
}

void AFP_FirstPersonCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFP_FirstPersonCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFP_FirstPersonCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFP_FirstPersonCharacter::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

FHitResult AFP_FirstPersonCharacter::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void AFP_FirstPersonCharacter::TryEnableTouchscreenMovement(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFP_FirstPersonCharacter::BeginTouch);
	PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFP_FirstPersonCharacter::EndTouch);
	PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AFP_FirstPersonCharacter::TouchUpdate);	
}


void AFP_FirstPersonCharacter::OnFireWeapon()
{
	_fireHeld = true;
}

void AFP_FirstPersonCharacter::OnFireWeaponRelease()
{
	_fireHeld = false;
	_currentWeaponComponent->OnFireEnd();
}

void AFP_FirstPersonCharacter::PickupWeapon()
{
	_currentWeaponComponent = _currentWeapon->GetWeaponComponent();
	_currentWeaponComponent->PickupWeapon(this);
	FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	_currentWeapon->AttachToComponent(Mesh1P, rules, TEXT("GripPoint"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Successful Raycast"));
}

void AFP_FirstPersonCharacter::DropWeapon()
{
	//if(_currentWeapon != nullptr)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Attempt Detach"));
	//	_currentWeaponComponent->DropWeapon();
	//	_currentWeaponComponent = nullptr;
	//	FDetachmentTransformRules rules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
	//	_currentWeapon->DetachFromActor(rules);
	//	_currentWeapon = nullptr;
	//}

	_currentWeaponComponent->DropWeapon();
	PlayerInventory->DropWeapon();
}

void AFP_FirstPersonCharacter::ReloadWeapon()
{
	if(_currentWeapon != nullptr)
	{
		IHasAmmo* AmmoRef = Cast<IHasAmmo>(_currentWeaponComponent);
		if(AmmoRef != nullptr)
		{
			AmmoRef->TryReload(_currentWeapon);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Start Reload"));
		}
	}
}

void AFP_FirstPersonCharacter::ThrowUtility()
{
	AGrenadeWeapon* GrenadeActor = GetWorld()->SpawnActor<AGrenadeWeapon>(_grenadeToSpawn, GetActorLocation() + FVector(0.0f, 0.0f, 50.0f), FRotator());
	GrenadeActor->SetInitialThrowForce((GetFirstPersonCameraComponent()->GetForwardVector() + (GetFirstPersonCameraComponent()->GetUpVector() / 2.0f)) * 100000.0f);
}

void AFP_FirstPersonCharacter::OnWeaponChanged(UWeaponItem* WeaponItem)
{
	if (WeaponItem != nullptr)
		FP_Gun->SetSkeletalMesh(Cast<UWeaponInfo>(WeaponItem->GetItemInfo())->WeaponSkeletalMesh);
	else
		FP_Gun->SetSkeletalMesh(nullptr);
	
	if (UGunItem* Gun = Cast<UGunItem>(WeaponItem))
	{
		_currentWeaponComponent->InitializeWeapon(Gun);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "NOT VALID");
	}
}

void AFP_FirstPersonCharacter::InteractWithObject()
{
	if(_lastLooked != nullptr)
	{
		if(_lastLooked->ActorHasTag(TEXT("Weapon")))
		{
			if (AItemActor* ItemActor = Cast<AItemActor>(_lastLooked))
				if(PlayerInventory->TryPickUpItem(ItemActor->GetItem()))
					ItemActor->Destroy();
		}
		else if(_lastLookedInterface)
		{
			_lastLookedInterface->Interact(this);
		}
	}
}

void AFP_FirstPersonCharacter::CastForInteractable(float DeltaTime)
{
	const FVector CameraPos = GetFirstPersonCameraComponent()->GetComponentLocation(), CameraDir = GetFirstPersonCameraComponent()->GetForwardVector();
	
	FHitResult Result;
	if (GetWorld()->LineTraceSingleByChannel(Result, CameraPos, CameraPos + (CameraDir * 250.f), ECollisionChannel::ECC_Visibility))
	{
		if(_lastLooked != Result.GetActor())
		{
			if(_lastLookedInterface != nullptr)
				_lastLookedInterface->EndHover();
			_lastLooked = Result.GetActor();
			if (IIInteractable* InteractActor = Cast<AItemActor>(Result.Actor))
			{
				_lastLookedInterface = InteractActor;
				InteractActor->StartHover();
			}
		}
	}
	else
	{
		_lastLooked = nullptr;
		if(_lastLookedInterface != nullptr)
			_lastLookedInterface->EndHover();
		_lastLookedInterface = nullptr;
	}
}



void AFP_FirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	_currentWeapon = Cast<AGunHostActor>(GunActorComponent->GetChildActor());
	_currentWeaponComponent = _currentWeapon->GetWeaponComponent();
	_currentWeaponComponent->SetParentMesh(FP_Gun);
	_currentWeaponComponent->PickupWeapon(this);
	
	//_currentWeapon = nullptr;
	//_currentWeaponComponent = nullptr; 
	_fireHeld = false;

	PlayerInventory->OnWeaponChangedEvent.AddDynamic(this, &AFP_FirstPersonCharacter::OnWeaponChanged);
	PlayerInventory->ComponentLoadComplete();

	_lastLooked = nullptr;
}

void AFP_FirstPersonCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CastForInteractable(DeltaSeconds);
	
	if(_fireHeld && _currentWeapon != nullptr)
	{
		_currentWeaponComponent->OnFire();
	}
}

