#include "Ai/BaseAiCharacter.h"
#include "Ai/AiCharacterData.h"
#include "Ai/BaseAiController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseAiCharacter::ABaseAiCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = ABaseAiController::StaticClass();
}

void ABaseAiCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnEnemyDied();
}

void ABaseAiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseAiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseAiCharacter::OnEnemyDied()
{
	GetMesh()->SetVisibility(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	GetCharacterMovement()->DisableMovement();

	dead = true;
}

void ABaseAiCharacter::SpawnEnemy(const FVector& RespawnPos)
{
	SetActorLocation(RespawnPos + FVector(0.f, 0.f, 300.f), false, nullptr, ETeleportType::TeleportPhysics);

	if (EnemyStats)
	{
		GetMesh()->SetSkeletalMesh(EnemyStats->SkeletalMeshes[FMath::RandRange(0, EnemyStats->SkeletalMeshes.Num() - 1)]);
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(EnemyStats->EnemyAnimBP);

		Health = EnemyStats->DefaultHealth * FMath::RandRange(0.f, EnemyStats->MaxDeviation);
		Damage = EnemyStats->DefaultDamage * FMath::RandRange(0.f, EnemyStats->MaxDeviation);

		if (EnemyStats->AiControllerClass)
		{
			if (GetController()->StaticClass() != EnemyStats->AiControllerClass) // IF AI Controller is different replace with expected controller. (Used for swapping Ai to bosses/other enemy types)
			{
				ABaseAiController* NewController = NewObject<ABaseAiController>(EnemyStats->AiControllerClass);
				NewController->Possess(this);
			}
		}
	}

	GetMesh()->SetVisibility(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	dead = false;
}
