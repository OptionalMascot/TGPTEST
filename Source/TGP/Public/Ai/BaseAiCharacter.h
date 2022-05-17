#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/HealthComponent.h"
#include "TGP/Public/AI/BaseAIController.h"

#include "BaseAiCharacter.generated.h"

class UAiCharacterData;

UCLASS()
class TGP_API ABaseAiCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", Meta = (AllowPrivateAccess = true)) UAiCharacterData* EnemyStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", Meta = (AllowPrivateAccess = true)) UHealthComponent* HealthComponent;
	
	float Damage;

protected:
	virtual void BeginPlay() override;

	ABaseAIController* baseAiController;

public:
	ABaseAiCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	bool IsAlive() const { return HealthComponent->health > 0.f; }

	UFUNCTION() void OnEnemyDied(AController* Causer);
	void SpawnEnemy(const FVector& RespawnPos);

	void SetHidden(bool bEnemyHidden);
};
