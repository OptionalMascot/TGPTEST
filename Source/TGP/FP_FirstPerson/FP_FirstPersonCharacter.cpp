#include "FP_FirstPersonCharacter.h"

#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/ActorChannel.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameInstance/BaseGameInstance.h"
#include "Weapons/GunHostActor.h"
#include "Weapons/WeaponComponent.h"
#include "Weapons/Interfaces/WeaponInterfaces.h"
#include "Inventory/PlayerInventory.h"
#include "Item/BaseItem.h"
#include "Item/ItemActor.h"
#include "Item/ItemInfo.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/HitscanWeaponComponent.h"
#include "Weapons/Throwables/GrenadeWeapon.h"
#include "Weapons/HealthComponent.h"
#include "Weapons/Projectiles/Projectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Throwables/ThrowableWeapon.h"

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
	FP_Gun->SetOnlyOwnerSee(false);			// Only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;		// Disallow mesh to cast dynamic shadows
	FP_Gun->CastShadow = false;			// Disallow mesh to cast other shadows
	FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));

	WeaponComponent = CreateDefaultSubobject<UHitscanWeaponComponent>(TEXT("GunComponent"));
	AddOwnedComponent(WeaponComponent);

	PlayerInventory = CreateDefaultSubobject<UPlayerInventory>(TEXT("PlayerInv"));
	AddOwnedComponent(PlayerInventory);

	_healthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	AddOwnedComponent(_healthComponent);

	// Set weapon damage and range
	WeaponRange = 5000.0f;
	WeaponDamage = 500000.0f;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	M_DefaultSpeed = 600.0f;
	M_SprintSpeed = 1350.0f;

	M_DefaultCameraSensitivity = 0.6;
	M_SniperSensitivity = 3.25;
	M_AimSensitivity = 1.5;
	M_CameraSensitivity = M_DefaultCameraSensitivity;

	CanFire = true;

	SwordCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Sword Collider"));
	SwordCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SwordCollider->SetCollisionResponseToAllChannels(ECR_Ignore);

	AimOffset = CreateDefaultSubobject<USceneComponent>(TEXT("Aim Weapon Location"));
	AimOffset->SetupAttachment(FirstPersonCameraComponent);

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

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFP_FirstPersonCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint",  IE_Released,  this, &AFP_FirstPersonCharacter::StopSprint);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AFP_FirstPersonCharacter::NewAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AFP_FirstPersonCharacter::NewStopAim);
	
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
	PlayerInputComponent->BindAxis("Turn", this, &AFP_FirstPersonCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFP_FirstPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this,&AFP_FirstPersonCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFP_FirstPersonCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("ChangeWeapon", this, &AFP_FirstPersonCharacter::ChangeWeapon);
}

void AFP_FirstPersonCharacter::ChangeWeapon(float Val)
{
	if (Val != 0.f)
	{
		if ((int)Val - 1 != PlayerInventory->GetSelectedWeaponSlot())
		{
			PlayerInventory->ChangeWeapon(EWeaponSlot(Val - 1.f));
			OnChangeSelectedWeapon(PlayerInventory->GetSelectedWeaponSlot());
		}
	}
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
	
	if(WeaponComponent != nullptr)
		WeaponComponent->OnFire();
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
	if(WeaponComponent->GetWeaponInfo()->WeaponType == EWeaponType::Sword)
	{
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

		if(!AnimInstance)
		{
			return;
		}

		if(IsMeleeAttacking)
		{
			return;
		}
			
		IsMeleeAttacking = true;
		// int AnimToPlay = FMath::RandRange(0,1);
		//
		// if(AnimToPlay == 0)
		// {
		// 	AnimInstance->Montage_Play(CombatMontage, 1.0f);
		// 	AnimInstance->Montage_JumpToSection("Melee1", CombatMontage);
		// }
		// else
		// {
		// 	AnimInstance->Montage_Play(CombatMontage, 1.0f);
		// 	AnimInstance->Montage_JumpToSection("Melee2", CombatMontage);
		// }
		
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
       	AnimInstance->Montage_JumpToSection("Melee1", CombatMontage);
		
		return;
	}
	_fireHeld = true;
}

void AFP_FirstPersonCharacter::OnFireWeaponRelease()
{
	_fireHeld = false;
	WeaponComponent->OnFireEnd();
}

void AFP_FirstPersonCharacter::PickupWeapon()
{
	WeaponComponent->PickupWeapon(this);
	FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
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

	WeaponComponent->DropWeapon();
	PlayerInventory->SrvDropWeapon(PlayerInventory->GetSelectedWeaponSlot());
	//PlayerInventory->DropWeapon();
}

void AFP_FirstPersonCharacter::ReloadWeapon()
{
	if(!WeaponComponent)
	{
		/*IHasAmmo* AmmoRef = Cast<IHasAmmo>(_currentWeaponComponent);
		if(AmmoRef != nullptr)
		{
			AmmoRef->TryReload(_currentWeapon);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Start Reload"));
		}*/
		//IHasAmmo* AmmoRef = Cast<IHasAmmo>(WeaponComponent);
		//if(AmmoRef != nullptr)
		//{
		//	AmmoRef->TryReload();
		//	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Start Reload"));
		//}
		
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Start Reload"));

	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	if(AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, WeaponComponent->GetWeaponInfo()->ReloadSpeed);
		AnimInstance->Montage_JumpToSection("Reload", CombatMontage);
	}
}

void AFP_FirstPersonCharacter::ThrowUtility()
{
	UThrowableItem* Throwable = PlayerInventory->GetSelectedUtility();

	if (Throwable)
	{
		APlayerController* _playerController = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0);
		FVector pos;
		FRotator rot;
		_playerController->GetPlayerViewPoint(pos, rot);
	
		AProjectile* ThrowableActor = GetWorld()->SpawnActor<AProjectile>(Cast<UThrowableInfo>(Throwable->GetItemInfo())->ThrowableBlueprint, GetActorLocation() + FVector(0.0f, 0.0f, 50.0f) + rot.Vector() * 100.0f, FRotator());
		ThrowableActor->Initialize(Cast<UThrowableInfo>(Throwable->GetItemInfo()));
		ThrowableActor->SetProjectileParameters(_playerController, rot.Vector(), 100000.0f);

		PlayerInventory->OnUseUtility();
	}
}

void AFP_FirstPersonCharacter::OnWeaponChanged(UWeaponItem* WeaponItem)
{
	if (WeaponItem != nullptr)
		FP_Gun->SetSkeletalMesh(Cast<UWeaponInfo>(WeaponItem->GetItemInfo())->WeaponSkeletalMesh);
	else
		FP_Gun->SetSkeletalMesh(nullptr);
	
	//if (HasAuthority())
	//{
	//	WeaponComponent->DropWeapon();
	//	
	//	WeaponComponent = NewObject<UWeaponComponent>(this, Cast<UGunInfo>(WeaponItem->GetItemInfo())->BaseWeaponClass, FName(WeaponItem->GetItemInfo()->ItemName));
	//	WeaponComponent->RegisterComponentWithWorld(GetWorld());
//
	//	WeaponComponent->PickupWeapon(this); // Assign player to component
	//	WeaponComponent->SetParentMesh(FP_Gun);
	//}
	
	if (UGunItem* Gun = Cast<UGunItem>(WeaponItem))
		WeaponComponent->InitializeWeapon(Gun);

	RequestWeaponMeshChange(PlayerInventory->GetSelectedWeaponSlot());
	AttachWeapon();
}

void AFP_FirstPersonCharacter::SrvHitScan_Implementation() // Rep is so scuffed within Gun actor I moved it here. I'm giving up on life at this point.
{
	FHitResult result;
	FVector CameraLoc;
	FRotator CameraRot;
	
	GetController()->GetPlayerViewPoint(CameraLoc, CameraRot);
	if(GetWorld()->LineTraceSingleByChannel(result, CameraLoc, CameraLoc + CameraRot.Vector() * 10000.0f, ECollisionChannel::ECC_Visibility)) // If hitting something
	{
		UWeaponItem* Wep = PlayerInventory->GetSelectedWeapon();
		UWeaponInfo* WepInfo =  Cast<UWeaponInfo>(Wep->GetItemInfo());

		if (WepInfo)
		{
			AActor* hit = result.GetActor(); // Get Actor
			UGameplayStatics::ApplyDamage(hit, WepInfo->Damage, GetController(), this, UDamageType::StaticClass());

			TestDebug();
		}
		
		//float dealtDamage = UGameplayStatics::ApplyDamage(hit, _weaponInfo->Damage, _parentController, GetOwner(), UDamageType::StaticClass()); // Attempt to apply damage
	}

	AttachWeapon();
}

void AFP_FirstPersonCharacter::SrvShootGun_Implementation()
{
	//Cast<AGunHostActor>(GunActorComponent->GetChildActor())->GetWeaponComponent()->SrvOnFire();

	WeaponComponent->SrvOnFire();
}

void AFP_FirstPersonCharacter::OnChangeSelectedWeapon_Implementation(int Slot)
{
	PlayerInventory->ChangeWeapon((EWeaponSlot)Slot, true, false);
}

void AFP_FirstPersonCharacter::OnPickUpItem_Implementation(AItemActor* ItemActor, int Slot)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::FromInt(Slot));
	
	if(PlayerInventory->TryPickUpItem(ItemActor->GetItem(), Slot))
		ItemActor->Destroy();
}

void AFP_FirstPersonCharacter::OnWeaponDropped_Implementation()
{
	//AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(PlayerInventory->GetItemActor(), GetActorLocation() + (GetOwner()->GetActorForwardVector() * 100.f), FRotator());
	//ItemActor->Initialize(PlayerInventory->GetSelectedWeapon());
//
	//PlayerInventory->DropWeapon();
}
 
void AFP_FirstPersonCharacter::ChangeWeaponMeshMulti_Implementation(int ItemId)
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
		return;
	
	if (UBaseGameInstance* GI = Cast<UBaseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
		if (UWeaponInfo* Info = Cast<UWeaponInfo>(GI->FindInfoUniqueId(ItemId)))
			FP_Gun->SetSkeletalMesh(Info->WeaponSkeletalMesh);
}

void AFP_FirstPersonCharacter::RequestWeaponMeshChange_Implementation(int Slot)
{
	PlayerInventory->ChangeWeapon((EWeaponSlot)Slot, true, false);

	if (UWeaponItem* Wep = PlayerInventory->GetSelectedWeapon())
	{
		ChangeWeaponMeshMulti(Wep->GetItemId());

		if (UGunItem* Gun = Cast<UGunItem>(Wep))
		{
			WeaponComponent = NewObject<UWeaponComponent>(this, Cast<UGunInfo>(Gun->GetItemInfo())->BaseWeaponClass, FName(Gun->GetItemInfo()->ItemName));
			WeaponComponent->RegisterComponentWithWorld(GetWorld());

			WeaponComponent->PickupWeapon(this); // Assign player to component
			WeaponComponent->SetParentMesh(FP_Gun);
			WeaponComponent->InitializeWeapon(Gun);
		}
	}
}

void AFP_FirstPersonCharacter::InteractWithObject()
{
	if(_lastLooked != nullptr)
	{
		if(_lastLooked->ActorHasTag(TEXT("Weapon")))
		{
			if (AItemActor* ItemActor = Cast<AItemActor>(_lastLooked))
					OnPickUpItem(ItemActor, PlayerInventory->GetSelectedWeaponSlot());
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

void AFP_FirstPersonCharacter::OnOverlapWithActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OVERLAP EVENT"));
}


void AFP_FirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	_healthComponent->health -= 80.0f;
	
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AFP_FirstPersonCharacter::OnOverlapWithActor);

	//_currentWeapon = Cast<AGunHostActor>(GunActorComponent->GetChildActor());
	//_currentWeaponComponent = _currentWeapon->GetWeaponComponent();
	WeaponComponent->SetParentMesh(FP_Gun);
	WeaponComponent->PickupWeapon(this);
		
	//_currentWeapon = nullptr;
	//_currentWeaponComponent = nullptr;

	if (PlayerInventory)
	{
		PlayerInventory->OnWeaponChangedEvent.AddDynamic(this, &AFP_FirstPersonCharacter::OnWeaponChanged);
		PlayerInventory->ComponentLoadComplete();
	}
	
	if (HasAuthority())
	{
		_fireHeld = false;
	}

	_lastLooked = nullptr;

	SwordCollider->OnComponentBeginOverlap.AddDynamic(this, &AFP_FirstPersonCharacter::MeleeDamage);
}

void AFP_FirstPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AFP_FirstPersonCharacter, _currentWeapon);
	//DOREPLIFETIME(AFP_FirstPersonCharacter, _currentWeaponComponent);
	//DOREPLIFETIME(AFP_FirstPersonCharacter, _fireHeld);
}

bool AFP_FirstPersonCharacter::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bUpdate = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	//bUpdate |= Channel->ReplicateSubobject(_currentWeapon, *Bunch, *RepFlags);
	//bUpdate |= Channel->ReplicateSubobject(WeaponComponent, *Bunch, *RepFlags);
 
	return bUpdate;
}

void AFP_FirstPersonCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (Controller)
	{
		CastForInteractable(DeltaSeconds);

		if(_fireHeld)
		{
			//if (HasAuthority())
			//	WeaponComponent->SrvOnFire();
			//else
			if (WeaponComponent->OnFire())
				SrvShootGun();
		}
	}
	
	//if(_fireHeld && _currentWeapon != nullptr)
	//{
	//	_currentWeaponComponent->OnFire();
	//	
	//	if (_currentWeaponComponent->bDidFire)
	//	{
	//		//SrvHitScan();
	//	}
	//}
}

void AFP_FirstPersonCharacter::LookUp(float inputValue)
{
	AddControllerPitchInput(inputValue * M_CameraSensitivity);
}

void AFP_FirstPersonCharacter::Turn(float inputValue)
{
	AddControllerYawInput(inputValue * M_CameraSensitivity);
}

void AFP_FirstPersonCharacter::SetWeaponTransformDefaults()
{
	WeaponDefaultLocation = Mesh1P->GetRelativeLocation();
	WeaponAimLocation = FirstPersonCameraComponent->GetComponentLocation() - FP_Gun->GetSocketLocation(FName("AimSocket"));
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, WeaponAimLocation.ToString());
	const float TempY = WeaponAimLocation.Y;
	WeaponAimLocation.Y = WeaponAimLocation.X;
	WeaponAimLocation.X = TempY;
	WeaponAimLocation.Z = WeaponDefaultLocation.Z;

	//WeaponAimLocation = WeaponDefaultLocation + WeaponAimLocation;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, FString::SanitizeFloat(GetActorRotation().Yaw));

	if(GetActorRotation().Yaw != 0)
	{
		WeaponAimLocation.X = WeaponAimLocation.X * FMath::Cos(GetActorRotation().Yaw) - WeaponAimLocation.X * FMath::Sin(GetActorRotation().Yaw);
		WeaponAimLocation.Y = WeaponAimLocation.Y * FMath::Sin(GetActorRotation().Yaw) + WeaponAimLocation.Y * FMath::Cos(GetActorRotation().Yaw);
	}
	

	WeaponDefaultRotation = Mesh1P->GetRelativeRotation();
	WeaponYawDiff = FirstPersonCameraComponent->GetComponentRotation().Yaw - FP_Gun->GetComponentRotation().Yaw + Mesh1P->GetComponentRotation().Yaw;

	if(WeaponDefaultRotation.Yaw != 0.0f)
	{
		WeaponYawDiff -= GetActorRotation().Yaw;
	}

	AimRotation = FRotator(WeaponDefaultRotation.Pitch + WeaponPitchDiff, WeaponDefaultRotation.Yaw + WeaponYawDiff, WeaponDefaultRotation.Roll);
}

void AFP_FirstPersonCharacter::AttachWeapon()
{
	switch (WeaponComponent->GetWeaponInfo()->WeaponType)
	{
	case EWeaponType::OneHand:
		{
			FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("PistolSocket"));
			break;
		}
	case EWeaponType::TwoHand:
		{
			if(FP_Gun->SkeletalMesh->GetName().Contains("SMG"))
			{
				FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("SubMachineSocket"));
			}
			else
			{
				FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("RifleSocket"));
			}
			break;
		}
	case EWeaponType::Sword:
		{
			FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("MeleeSocket"));
			SwordCollider->AttachToComponent(FP_Gun, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Collider"));
			break;
		}
	default:
		break;
	}
	SetAnimation();
}

void AFP_FirstPersonCharacter::Sprint()
{
	if(!IsReloading)
	{
		IsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = M_SprintSpeed;
	}
}

void AFP_FirstPersonCharacter::StopSprint()
{
	IsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = M_DefaultSpeed;
}

void AFP_FirstPersonCharacter::NewAim()
{
	if(WeaponComponent->GetWeaponInfo()->WeaponType == EWeaponType::Sword)
	{
		if(IsAiming)
		{
			NewStopAim();
		}
		
		return;
	}
	
	IsAiming = true;
	if(FP_Gun->SkeletalMesh->GetName().Contains("Sniper"))
	{
		M_CameraSensitivity = M_DefaultCameraSensitivity/M_SniperSensitivity;
		FirstPersonCameraComponent->SetFieldOfView(25.0f);
	}
	else
	{
		M_CameraSensitivity = M_DefaultCameraSensitivity/M_AimSensitivity;
		FirstPersonCameraComponent->SetFieldOfView(85.0f);
	}
	
	Mesh1P->SetHiddenInGame(true);
	FP_Gun->AttachToComponent(AimOffset, FAttachmentTransformRules::SnapToTargetIncludingScale);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FP_Gun->SkeletalMesh->GetName());
	
	switch (WeaponComponent->GetWeaponInfo()->WeaponType)
	{
	case EWeaponType::TwoHand:
		if(FP_Gun->SkeletalMesh->GetName().Contains("SMG"))
		{
			UE_LOG(LogTemp, Warning, TEXT("SMGGGG"));
			AdjustToIrons();
			break;
		}

		if(FP_Gun->SkeletalMesh->GetName().Contains("Sniper"))
		{
			break;
		}
		
		AdjustToSight();
		break;
	case EWeaponType::OneHand:
		
		if(FP_Gun->SkeletalMesh->GetName().Contains("Machine"))
		{
			AdjustToSight();
			break;
		}
		
		AdjustToIrons();
		break;
	default:
		break;
	}
}

void AFP_FirstPersonCharacter::NewStopAim()
{
	if(IsAiming)
	{
		Mesh1P->SetHiddenInGame(false);
		IsAiming = false;
		M_CameraSensitivity = M_DefaultCameraSensitivity;
		FirstPersonCameraComponent->SetFieldOfView(100.0f);
		AttachWeapon();
	}
}

void AFP_FirstPersonCharacter::ResetAim()
{
	Mesh1P->SetHiddenInGame(false);
	IsAiming = false;
	M_CameraSensitivity = M_DefaultCameraSensitivity;
	FirstPersonCameraComponent->SetFieldOfView(100.0f);
}

void AFP_FirstPersonCharacter::SwitchWeapon()
{
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

	if(!AnimInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("NoAnimations"));
		return;
	}
	
	switch (WeaponComponent->GetWeaponInfo()->WeaponType)
	{
	case EWeaponType::TwoHand:
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection("SwitchRifle", CombatMontage);
		break;
	case EWeaponType::OneHand:
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection("SwitchPistol", CombatMontage);
		break;
	case EWeaponType::Sword:
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection("SwitchSword", CombatMontage);
		break;
	default:
		break;
	}
}

void AFP_FirstPersonCharacter::Reload()
{
	IHasAmmo* AmmoRef = Cast<IHasAmmo>(WeaponComponent);
	if(AmmoRef != nullptr)
	{
		AmmoRef->TryReload();
		CanFire = true;
	}
}

void AFP_FirstPersonCharacter::PlayFireAnim()
{
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

	if(!AnimInstance)
	{
		return;
	}
	
	float AnimLegnth = 0.0f;
	float GunFireRate = 0.0f;
	float AdjustedPlayRate =1.0f;

	switch (WeaponComponent->GetWeaponInfo()->WeaponType)
	{
	case EWeaponType::TwoHand:
		AnimLegnth = CombatMontage->GetSectionLength(CombatMontage->GetSectionIndex("RifleFire"));
		GunFireRate = WeaponComponent->GetWeaponInfo()->AttackRate ;
		AdjustedPlayRate = (GunFireRate * 60.0f)/AnimLegnth;

		AnimInstance->Montage_Play(CombatMontage, AdjustedPlayRate);
		AnimInstance->Montage_JumpToSection("RifleFire", CombatMontage);
		break;
	case EWeaponType::OneHand:
		AnimLegnth = CombatMontage->GetSectionLength(CombatMontage->GetSectionIndex("PistolFire"));
		GunFireRate = WeaponComponent->GetWeaponInfo()->AttackRate;
		AdjustedPlayRate = (GunFireRate * 60.0f)/AnimLegnth;

		AnimInstance->Montage_Play(CombatMontage, AdjustedPlayRate);
		AnimInstance->Montage_JumpToSection("PistolFire", CombatMontage);
		break;
	default:
		break;
	}

}

void AFP_FirstPersonCharacter::SwordColliderOn()
{
	SwordCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AFP_FirstPersonCharacter::SwordColliderOff()
{
	SwordCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void AFP_FirstPersonCharacter::MeleeDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor->IsA(AFP_FirstPersonCharacter::StaticClass()))
	{
		SwordColliderOff();
		UGameplayStatics::ApplyDamage(OtherActor, WeaponComponent->GetWeaponInfo()->Damage, GetController(), this, UDamageType::StaticClass());
	}
}
