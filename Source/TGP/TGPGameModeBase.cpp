#include "TGPGameModeBase.h"
#include "DrawDebugHelpers.h"
#include "FP_FirstPerson/FP_FirstPersonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Ai/BaseAiCharacter.h"
#include "Ai/AiCharacterData.h"

ATGPGameModeBase::ATGPGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATGPGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyPool.Reserve(MaxEnemies);

	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;

	for (uint8 i = 0; i < MaxEnemies; i++) // Init Pool
		EnemyPool.Add(GetWorld()->SpawnActor<ABaseAiCharacter>(AiActorClass ? AiActorClass : ABaseAiCharacter::StaticClass(), FVector() + (FVector(100.f, 0.f, 0.f) * i), FRotator(), SpawnParams));

	GetWorld()->GetTimerManager().SetTimer(RoundCooldownHandler, this, &ATGPGameModeBase::BeginRound, CooldownBetweenRounds, false);
}

void ATGPGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnemiesToSpawn > 0)
	{
		SpawnerTimer += DeltaSeconds;

		if (SpawnerTimer >= SpawnTimer)
		{
			const int WaveCount = EnemiesToSpawn < MaxEnemiesPerSpawnWave ? EnemiesToSpawn : MaxEnemiesPerSpawnWave;
			
			for (uint8 i = 0; i < WaveCount; i++)
				if(!TrySpawnEnemy())
					break;

			SpawnerTimer = 0.f;
		}	
	}
	else if (EnemiesAlive <= 0 && !bPendingRoundRestart)
	{
		bPendingRoundRestart = true;
		SpawnerTimer = 0.f;
		EndRound();
	}
}

void ATGPGameModeBase::BeginRound()
{
	EnemiesToSpawn = (int)((SpawnIncreaseExponential * CurrentRound) * 24);
	SpawnerTimer = 0.f;
	bPendingRoundRestart = false;
}

void ATGPGameModeBase::EndRound()
{
	CurrentRound++;
	GetWorld()->GetTimerManager().SetTimer(RoundCooldownHandler, this, &ATGPGameModeBase::BeginRound, CooldownBetweenRounds, false); // Begin Cooldown between rounds
}

bool ATGPGameModeBase::TrySpawnEnemy()
{
	const int AvailableIndex = FindAvailableEnemy();

	if (AvailableIndex == -1)
		return false;
	
	for (int i = 0; i < 100; i++)
	{
		const FVector AttemptedSpawnPoint = FVector(FMath::RandRange(SpawnMinRange.X, SpawnMaxRange.X), FMath::RandRange(SpawnMinRange.Y, SpawnMaxRange.Y), 3000.f);

		DrawDebugLine(GetWorld(),AttemptedSpawnPoint, AttemptedSpawnPoint + (FVector::DownVector * 6000.f), FColor::Red, false, 1.f);
		
		FHitResult Result;
		if (GetWorld()->LineTraceSingleByChannel(Result, AttemptedSpawnPoint, AttemptedSpawnPoint + (FVector::DownVector * 6000.f), ECollisionChannel::ECC_Visibility))
		{
			if (Result.Actor->Tags.Contains("Ground"))
			{
				bool bCanSpawn = false;

				for (int j = 0; j < GetNumPlayers(); j++)
				{
					if (const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), j))
					{
						if (const AFP_FirstPersonCharacter* Pawn = Cast<AFP_FirstPersonCharacter>(Controller->GetPawn()))
						{
							const FVector PawnLoc = Pawn->GetActorLocation(), SpawnLoc = Result.Location + FVector(0.f, 0.f, 44.f);
							
							if (FVector::Dist(SpawnLoc, PawnLoc) >= MinSpawnDistFromPlayer)
							{
								FVector SpawnDir = PawnLoc - SpawnLoc;
								SpawnDir.Normalize();

								DrawDebugLine(GetWorld(),PawnLoc, SpawnLoc, FColor::Blue, false, 1.f);
								
								if (CheckLineOfSight(PawnLoc, SpawnLoc) || IsLookingAtDir(Pawn->GetActorForwardVector(), SpawnDir))
									bCanSpawn = true;
							}
							else
							{
								bCanSpawn = false;
								break;
							}
						}
					}
				}

				if (bCanSpawn)
				{
					SpawnEnemy(AvailableIndex, Result.Location);
					return true;
				}
			}
		}
	}

	return false;
}

int ATGPGameModeBase::FindAvailableEnemy()
{
	for	(uint8 i = 0; i < EnemyPool.Num(); i++)
		if (!EnemyPool[i]->IsAlive())
			return i;

	return -1;
}

void ATGPGameModeBase::OnEnemyKilled(ABaseAiCharacter* Enemy)
{
	EnemiesAlive--;
}

void ATGPGameModeBase::SpawnEnemy(uint8 EnemyIndex, const FVector& Position) // TODO: RESET OTHER PROPERTIES LIKE HEALTH
{
	if (EnemyIndex < EnemyPool.Num())
	{
		EnemyPool[EnemyIndex]->SpawnEnemy(Position);
	
		EnemiesAlive++;
		EnemiesToSpawn--;
	}
}

bool ATGPGameModeBase::IsLookingAtDir(const FVector& PawnDir, const FVector& DirToPoint) const
{
	return FVector::DotProduct(PawnDir, DirToPoint) > 0.f;
}

void ATGPGameModeBase::DEBUG_KILL_ENEMY()
{
	for (uint8 i = 0; i < EnemyPool.Num(); i++)
	{
		if (EnemyPool[i]->IsAlive())
		{
			EnemyPool[i]->OnEnemyDied();
			OnEnemyKilled(EnemyPool[i]);
			break;
		}
	}
}

bool ATGPGameModeBase::CheckLineOfSight(const FVector& PawnLoc, const FVector& SpawnPoint) const
{
	FHitResult Result;
	return GetWorld()->LineTraceSingleByChannel(Result, PawnLoc, SpawnPoint, ECollisionChannel::ECC_Visibility);
}
