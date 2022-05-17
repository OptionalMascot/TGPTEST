// Fill out your copyright notice in the Description page of Project Settings.


#include "Aki/AkiCharacter.h"
#include "Camera/CameraComponent.h"

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
}

// Called when the game starts or when spawned
void AAkiCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

}

