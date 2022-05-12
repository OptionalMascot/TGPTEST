// Copyright Epic Games, Inc. All Rights Reserved.

#include "TGPGameModeBase.h"

#include "DrawDebugHelpers.h"
#include "FP_FirstPerson/FP_FirstPersonCharacter.h"
#include "Kismet/GameplayStatics.h"

ATGPGameModeBase::ATGPGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;
}

void ATGPGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//EnemyPool.Reserve(MaxEnemies);

	//for (uint8 i = 0; i < MaxEnemies; i++)
	//	EnemyPool.Add(GetWorld()->SpawnActor(RegularZombieClass));

	//BeginRound();
}


void ATGPGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnemiesAlive > 0 || EnemiesToSpawn > 0)
	{
		SpawnerTimer += DeltaSeconds;

		if (SpawnerTimer >= SpawnTimer)
		{
			for (uint8 i = 0; i < MaxEnemiesPerSpawnWave; i++)
				if(!TrySpawnEnemy())
					break;

			SpawnerTimer = 0.f;
		}
	}
}

void ATGPGameModeBase::BeginRound()
{
	EnemiesToSpawn = (int)((SpawnExponential * CurrentRound) * 24);
	SpawnerTimer = 0.f;
}

void ATGPGameModeBase::EndRound()
{
	CurrentRound++;
}

bool ATGPGameModeBase::TrySpawnEnemy()
{
	const uint8 AvailableIndex = FindAvailableEnemy();

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
								
								if (IsLookingAtDir(Pawn->GetActorForwardVector(), SpawnDir) && CheckLineOfSight(PawnLoc, SpawnLoc))
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
					DrawDebugBox(GetWorld(), Result.Location, FVector(30.f), FColor::Green, false, 1.f, 0, 5);
					return true;
				}
			}
		}
	}

	return false;
}

uint8 ATGPGameModeBase::FindAvailableEnemy()
{
	// TODO: IMPL when Ai is added

	return -1;
}

void ATGPGameModeBase::OnEnemyKilled(AActor* Enemy)
{
	
}

void ATGPGameModeBase::SpawnEnemy(uint8 EnemyIndex, const FVector& Position) // TODO: RESET OTHER PROPERTIES LIKE HEALTH
{
	AActor* Enemy = EnemyPool[EnemyIndex];
	
	Enemy->SetActorLocation(Position);
}

bool ATGPGameModeBase::IsLookingAtDir(const FVector& PawnDir, const FVector& DirToPoint) const
{
	return FVector::DotProduct(PawnDir, DirToPoint) > 0.f;
}

bool ATGPGameModeBase::CheckLineOfSight(const FVector& PawnLoc, const FVector& SpawnPoint) const
{
	FHitResult Result;
	return GetWorld()->LineTraceSingleByChannel(Result, PawnLoc, SpawnPoint, ECollisionChannel::ECC_Visibility);
}