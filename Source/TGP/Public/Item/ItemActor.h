#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UBaseItem;

UCLASS()
class TGP_API AItemActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UStaticMeshComponent* ItemMesh;
	UPROPERTY() UBaseItem* DefinedItem;

protected:
	virtual void BeginPlay() override;

public:
	AItemActor();
	virtual void Tick(float DeltaTime) override;

	void Initialize(UBaseItem* Item);
};
