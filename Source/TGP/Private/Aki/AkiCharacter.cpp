// Fill out your copyright notice in the Description page of Project Settings.


#include "Aki/AkiCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
AAkiCharacter::AAkiCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetRootComponent());

	GetMesh()->SetupAttachment(Camera);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMesh->SetupAttachment(GetMesh());

	AimLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Location"));
	AimLocation->SetupAttachment(Camera);

	M_CameraSensitivity = 0.6f; //Try keep between 0-1 otherwise sensitivity gets out of hand

	IsSprinting = false;
	M_DefaultSpeed = 600.0f;
	M_SprintSpeed = 1350.0f;

	IsAiming = false;
	IsReloading = false;

	M_GunReloadSpeed = 2.5f;
	
}

// Called when the game starts or when spawned
void AAkiCharacter::BeginPlay()
{
	Super::BeginPlay();

	WeaponDefaultLocation = GetMesh()->GetRelativeLocation();
	WeaponLocationOffset = WeaponDefaultLocation + (AimLocation->GetComponentLocation() - WeaponMesh->GetSocketLocation(FName("AimSocket")));
}

// Called every frame
void AAkiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAkiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Camera Controls
	PlayerInputComponent->BindAxis("LookUp", this, &AAkiCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AAkiCharacter::Turn);

	//PlayerMovement
	PlayerInputComponent->BindAxis("MoveForward", this, &AAkiCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAkiCharacter::MoveRight);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AAkiCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AAkiCharacter::StopSprint);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AAkiCharacter::BeginAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AAkiCharacter::EndAim);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AAkiCharacter::Reload);

}

void AAkiCharacter::LookUp(float inputValue)
{
	FRotator CameraRotation = Camera->GetComponentRotation();
	CameraRotation.Pitch = FMath::Clamp(CameraRotation.Pitch - (inputValue * M_CameraSensitivity), -60.0f, 60.0f);

	Camera->SetWorldRotation(CameraRotation);
}

void AAkiCharacter::Turn(float inputValue)
{
	AddControllerYawInput(inputValue * M_CameraSensitivity);
}

void AAkiCharacter::MoveForward(float inputValue)
{
	GetMovementComponent()->AddInputVector(GetActorForwardVector() * inputValue);
}

void AAkiCharacter::MoveRight(float inputValue)
{
	GetMovementComponent()->AddInputVector(GetActorRightVector() * inputValue);
}

void AAkiCharacter::Sprint()
{
	if(!IsReloading)
	{
		IsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = M_SprintSpeed;
	}
}

void AAkiCharacter::StopSprint()
{
	IsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = M_DefaultSpeed;
}

void AAkiCharacter::BeginAim()
{
	IsAiming = true;
	Aim();
}

void AAkiCharacter::EndAim()
{
	IsAiming = false;
	StopAim();
}

void AAkiCharacter::Reload()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance)
	{
		IsReloading = true;
		StopSprint();
		AnimInstance->Montage_Play(CombatMontage, M_GunReloadSpeed);
		AnimInstance->Montage_JumpToSection(FName("RifleReload"), CombatMontage);
	}
}

void AAkiCharacter::ReloadFinished()
{
	IsReloading = false;
}


