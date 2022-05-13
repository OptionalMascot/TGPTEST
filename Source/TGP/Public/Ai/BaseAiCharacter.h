#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/Interfaces/WeaponInterfaces.h"
#include "BaseAiCharacter.generated.h"

class UAiCharacterData;
class UHealthComponent;

UCLASS()
class TGP_API ABaseAiCharacter : public ACharacter, public IHealth
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", Meta = (AllowPrivateAccess = true)) UAiCharacterData* EnemyStats;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", Meta = (AllowPrivateAccess = true)) UHealthComponent* HealthComponent;

	float Health;
	float Damage;

protected:
	virtual void BeginPlay() override;

public:
	ABaseAiCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	bool IsAlive() const { return !dead; }

	void OnEnemyDied();
	void SpawnEnemy(const FVector& RespawnPos);
};
