#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInventory.generated.h"

class UItemContainer;
class UWeaponItem;
class UThrowableItem;

enum EWeaponSlot : uint8
{
	Primary = 0,
	Secondary = 1
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TGP_API UPlayerInventory : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventorySettings", Meta = (AllowPrivateAccess = true)) uint8 MaxUtilityAmount = 4;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventorySettings", Meta = (AllowPrivateAccess = true)) uint8 MaxConsumableAmount = 4;
	
	UPROPERTY() UItemContainer* UtilityContainer;
	UPROPERTY() UItemContainer* ConsumableContainer;

	UPROPERTY() UWeaponItem* PrimaryWeapon;
	UPROPERTY() UWeaponItem* SecondaryWeapon;

	EWeaponSlot SelectedWeapon;
	
	uint8 SelectedUtilitySlot = 0;

	void TryFindAndSelectValidUtility();
	
protected:
	virtual void BeginPlay() override;

public:
	UPlayerInventory();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool AddUtility(UThrowableItem* ThrowableItem) const;
	void SelectUtility(uint8 Slot);

	void PickUpWeapon(UWeaponItem* WeaponItem);
	void ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate = false);
	void DropWeapon();
};
