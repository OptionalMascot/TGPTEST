#include "Ai/BaseAiCharacter.h"
#include "Ai/AiCharacterData.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TGP/TGPGameModeBase.h"

ABaseAiCharacter::ABaseAiCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AController::StaticClass();
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AddOwnedComponent(HealthComponent);
}

void ABaseAiCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->onComponentDead.AddDynamic(this, &ABaseAiCharacter::OnEnemyDied);
	baseAiController = Cast<ABaseAIController>(GetController());
	baseAiController->RunBT();
}

void ABaseAiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseAiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseAiCharacter::OnEnemyDied(AController* Causer)
{
	SetHidden(true);

	if (ATGPGameModeBase* GM = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
		GM->OnEnemyKilled(this);
}

void ABaseAiCharacter::SpawnEnemy(const FVector& RespawnPos)
{
	SetActorLocation(RespawnPos + FVector(0.f, 0.f, 300.f), false, nullptr, ETeleportType::TeleportPhysics);

	if (EnemyStats)
	{
		GetMesh()->SetSkeletalMesh(EnemyStats->SkeletalMeshes[FMath::RandRange(0, EnemyStats->SkeletalMeshes.Num() - 1)]);
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(EnemyStats->EnemyAnimBP);

		HealthComponent->health = EnemyStats->DefaultHealth + (EnemyStats->DefaultHealth * FMath::RandRange(-EnemyStats->MaxDeviation, EnemyStats->MaxDeviation));
		Damage = EnemyStats->DefaultDamage + (EnemyStats->DefaultDamage * FMath::RandRange(0.f, EnemyStats->MaxDeviation));

		/*if (EnemyStats->AiControllerClass)
		{
			if (GetController()->StaticClass() != EnemyStats->AiControllerClass) // IF AI Controller is different replace with expected controller. (Used for swapping Ai to bosses/other enemy types)
			{
				AController* NewController = NewObject<AController>(EnemyStats->AiControllerClass);
				NewController->Possess(this);
			}
		}*/	
	}

	SetHidden(false);
}

void ABaseAiCharacter::SetHidden(bool bEnemyHidden)
{
	GetMesh()->SetVisibility(!bEnemyHidden);
	GetCapsuleComponent()->SetCollisionEnabled(bEnemyHidden ? ECollisionEnabled::Type::NoCollision : ECollisionEnabled::Type::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(bEnemyHidden ? EMovementMode::MOVE_None : EMovementMode::MOVE_Walking);

	if (bEnemyHidden)
		GetCharacterMovement()->DisableMovement();

	baseAiController->ChangeAIControllerStatus(!bEnemyHidden);
	
}
