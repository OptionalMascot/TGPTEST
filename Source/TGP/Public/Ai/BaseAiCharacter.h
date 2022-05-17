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

	UPROPERTY() ABaseAIController* baseAiController;

public:
	ABaseAiCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	bool IsAlive() const { return HealthComponent->health > 0.f; }

	UFUNCTION() void OnEnemyDied(AController* Causer);
	void SpawnEnemy(const FVector& RespawnPos);

	void SetHidden(bool bEnemyHidden);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attack") class USphereComponent* RightHandCollider;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attack") class UCapsuleComponent* LeftArmCollider;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations") class UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations") class UAnimMontage* DeathMontage;

	UFUNCTION(BlueprintCallable) void Attack();
	UFUNCTION(BlueprintCallable) void Die();

	UFUNCTION() void LHHitPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	UFUNCTION() void RHHitPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION(BlueprintCallable) void LeftColliderOn();
	UFUNCTION(BlueprintCallable) void LeftColliderOff();
	
	UFUNCTION(BlueprintCallable) void RightColliderOn();
	UFUNCTION(BlueprintCallable) void RightColliderOff();
};
