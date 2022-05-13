#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UItemInfo;
class UBaseItem;
class UBoxComponent;
class UPointLightComponent;

UCLASS()
class TGP_API AItemActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UStaticMeshComponent* ItemMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) USkeletalMeshComponent* ItemSkeletalMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UPointLightComponent* ItemPointLight;
	
	UPROPERTY(BlueprintReadOnly, Category = "Item", Meta = (AllowPrivateAccess = true)) UBaseItem* DefinedItem;

protected:
	virtual void BeginPlay() override;

public:
	AItemActor();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable) void Initialize(UBaseItem* Item);
	UBaseItem* GetItem() const { return DefinedItem; }
};
