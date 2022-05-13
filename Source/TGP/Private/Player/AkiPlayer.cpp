// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AkiPlayer.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAkiPlayer::AAkiPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FirstPersonCamera->SetupAttachment(GetRootComponent());

	GetMesh()->SetupAttachment(FirstPersonCamera);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	WeaponMesh->SetupAttachment(GetMesh());

	AimLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MeshAimLocation"));
	AimLocation->SetupAttachment(FirstPersonCamera);

	DefaultFieldOfView = 100.0f;
	
	

	

	M_CameraSensitivity = 0.6f;

	IsSprinting = false;

	GetCharacterMovement()->AirControl = 1.0f;

}

// Called when the game starts or when spawned
void AAkiPlayer::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = M_DefaultSpeed;
	
	WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("RifleSocket"));

	DefaultMeshLocation = GetMesh()->GetRelativeLocation();
	GetMesh()->HideBone(5, EPhysBodyOp::PBO_None);

	IsSprinting = false;
	IsAiming = false;
}

// Called every frame
void AAkiPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAkiPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Player Movement Inputs
	PlayerInputComponent->BindAxis("MoveForward", this, &AAkiPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAkiPlayer::MoveRight);

	//Player Camera Control Inputs
	PlayerInputComponent->BindAxis("Turn", this, &AAkiPlayer::LookAround);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AAkiPlayer::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AAkiPlayer::StopSprinting);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AAkiPlayer::ADS);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &AAkiPlayer::StopADS);

}

void AAkiPlayer::MoveForward(float inputValue)
{
	if(inputValue != 0)
	{
		M_MovingForward = true;
		GetCharacterMovement()->AddInputVector(GetActorForwardVector() * inputValue);
	}
	else
	{
		M_MovingForward = false;
	}

	if(!M_MovingForward && !M_MovingRight)
	{
		StopSprinting();
	}
}

void AAkiPlayer::MoveRight(float inputValue)
{
	if(inputValue != 0)
	{
		M_MovingRight = true;
		GetCharacterMovement()->AddInputVector(GetActorRightVector() * inputValue);
	}
	else
	{
		M_MovingRight = false;
	}
}

void AAkiPlayer::LookAround(float inputValue)
{
	AddControllerYawInput(inputValue * M_CameraSensitivity);
}

void AAkiPlayer::LookUp(float inputValue)
{
}

void AAkiPlayer::Sprint()
{
	if(GetVelocity().Size() > 0 && !IsAiming)
	{
		IsSprinting = true;

		GetCharacterMovement()->MaxWalkSpeed = 1700.0f;
	}
	else
	{
		StopSprinting();
	}
}

void AAkiPlayer::StopSprinting()
{
	IsSprinting = false;

	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void AAkiPlayer::ADS()
{
	IsAiming = true;
	StopSprinting();
	BeginAimTransisiton();
	AdjustFOV(1.25f);
	
}

void AAkiPlayer::StopADS()
{
	IsAiming = false;
	BeginUnAimTransisiton();
	AdjustFOV(1.0f);
}

void AAkiPlayer::AdjustFOV(float ZoomLevel)
{
	float NewFieldOfView = DefaultFieldOfView - (DefaultFieldOfView * ZoomLevel - DefaultFieldOfView);

	FirstPersonCamera->SetFieldOfView(NewFieldOfView);
}


