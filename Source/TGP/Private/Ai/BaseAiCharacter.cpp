#include "Ai/BaseAiCharacter.h"
#include "Ai/AiCharacterData.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TGP/TGPGameModeBase.h"

ABaseAiCharacter::ABaseAiCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = ABaseAIController::StaticClass();
	
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
	// SetHidden(true);
	//
	// if (ATGPGameModeBase* GM = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	// 	GM->OnEnemyKilled(this);

	UE_LOG(LogTemp, Warning, TEXT("Zombie Died"));
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	if(AnimInstance)
	{
		if(!GetMesh()->GetAnimInstance()->Montage_IsPlaying(DeathMontage))
		{
			int DeathAnim = FMath::RandRange(0, DeathMontage->CompositeSections.Num() - 1);
			FName SectionName = DeathMontage->GetSectionName(DeathAnim);
	
			AnimInstance->Montage_Play(DeathMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Animation"));
	}
	
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

void ABaseAiCharacter::Attack()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance)
	{
		if(!GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage))
		{
			int AttackAnim = FMath::RandRange(0, AttackMontage->CompositeSections.Num() - 1);
			FName SectionName = AttackMontage->GetSectionName(AttackAnim);

			switch (AttackAnim)
			{
				case 0:
				{
					AnimInstance->Montage_Play(AttackMontage, 1.35f);
					AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
					break;
				}
				case 2:
				{
					AnimInstance->Montage_Play(AttackMontage, 1.15f);
					AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
					break;
				}
				case 3:
				{
					AnimInstance->Montage_Play(AttackMontage, 0.75f);
					AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
					break;
				}
				default:
				{
						AnimInstance->Montage_Play(AttackMontage, 1.0f);
						AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
					break;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Animation"));
	}
}

void ABaseAiCharacter::Die()
{
	SetHidden(true);

	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	if (ATGPGameModeBase* GM = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
		GM->OnEnemyKilled(this);
}
