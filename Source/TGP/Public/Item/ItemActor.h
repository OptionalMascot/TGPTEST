#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UItemInfo;
class UBaseItem;
class UBoxComponent;
class UPointLightComponent;
class UWeaponInfo;
class UWeaponStatUIWidget;

UCLASS()
class TGP_API AItemActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UStaticMeshComponent* ItemMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) USkeletalMeshComponent* ItemSkeletalMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) UPointLightComponent* ItemPointLight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = true)) class UWidgetComponent* StatWidget;

	UPROPERTY() APlayerController* _playerController;
	
	UPROPERTY(BlueprintReadOnly, Category = "Item", Meta = (AllowPrivateAccess = true)) UBaseItem* DefinedItem;

protected:
	virtual void BeginPlay() override;

	virtual void WidgetBillboard();

	void InitialiseWidgetText(const UWeaponInfo* info);

	
	
public:
	AItemActor();
	virtual void Tick(float DeltaTime) override;

	void LightColourSetup(const UWeaponInfo* info) const;

	USkeletalMeshComponent* GetSkeletalMesh() { return ItemSkeletalMesh; }
	
	UFUNCTION(BlueprintCallable) void Initialize(UBaseItem* Item);
	UBaseItem* GetItem() const { return DefinedItem; }
};
