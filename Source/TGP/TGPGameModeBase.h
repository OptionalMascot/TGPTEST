#pragma once

#include "Item/BaseItem.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameInstance/BaseGameInstance.h"
#include "TGPGameModeBase.generated.h"

UCLASS()
class TGP_API ATGPGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY() TArray<AActor*> EnemyPool; // REPLACE WITH AI CLASS

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", Meta = (AllowPrivateAccess = true)) FVector SpawnMinRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", Meta = (AllowPrivateAccess = true)) FVector SpawnMaxRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", Meta = (AllowPrivateAccess = true)) float MinSpawnDistFromPlayer = 300.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", Meta = (AllowPrivateAccess = true)) float SpawnExponential = 0.15f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", Meta = (AllowPrivateAccess = true)) float SpawnTimer = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies", Meta = (AllowPrivateAccess = true)) TSubclassOf<AActor> RegularZombieClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies", Meta = (AllowPrivateAccess = true)) TSubclassOf<AActor> SpitterZombieClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies", Meta = (AllowPrivateAccess = true)) TSubclassOf<AActor> BossZombieClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies", Meta = (AllowPrivateAccess = true)) uint8 MaxEnemies = 24;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies", Meta = (AllowPrivateAccess = true)) uint8 MaxEnemiesPerSpawnWave = 4;

	uint8 EnemiesAlive = 0;
	
	uint32 EnemiesToSpawn = 0;
	uint32 CurrentRound = 1;

	float SpawnerTimer = 0.f;

	void BeginRound();
	void EndRound();

	UFUNCTION(CallInEditor) bool TrySpawnEnemy();

	uint8 FindAvailableEnemy();
	void SpawnEnemy(uint8 EnemyIndex, const FVector& Position);

	bool CheckLineOfSight(const FVector& PawnLoc, const FVector& SpawnPoint) const;
	bool IsLookingAtDir(const FVector& PawnDir, const FVector& DirToPoint) const;

	protected:
	virtual void BeginPlay() override;

public:
	ATGPGameModeBase();

	virtual void Tick(float DeltaSeconds) override;

	void OnEnemyKilled(AActor* Enemy); // TODO: REPLACE WITH AI CLASS
	
	template<class T>
	T* CreateItemByShortName(const FString& ItemShortName, const int Amount = 1);

	template<class T>
	T* CreateItemByUniqueId(int UniqueId, const int Amount = 1);
};

template <class T>
T* ATGPGameModeBase::CreateItemByShortName(const FString& ItemShortName, const int Amount)
{
	if (TIsDerivedFrom<T, UBaseItem>::IsDerived)
	{
		UItemInfo* Info = Cast<UBaseGameInstance>(GetGameInstance())->FindInfoShortName(ItemShortName);

		if (Info != nullptr)
		{
			T* NewItem = NewObject<T>();
			Cast<UBaseItem>(NewItem)->Init(Info, Amount);
		}
	}
	
	return nullptr;
}

template <class T>
T* ATGPGameModeBase::CreateItemByUniqueId(int UniqueId, const int Amount)
{
	UItemInfo* Info = Cast<UBaseGameInstance>(GetGameInstance())->FindInfoUniqueId(UniqueId);

	if (Info != nullptr)
	{
		T* NewItem = NewObject<T>();
		Cast<UBaseItem>(NewItem)->Init(Info, Amount);

		return NewItem;
	}
	
	return nullptr;
}