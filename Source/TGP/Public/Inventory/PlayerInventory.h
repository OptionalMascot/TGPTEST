#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInventory.generated.h"

class UItemContainer;
class UWeaponItem;
class UThrowableItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, UWeaponItem*, WeaponItem);

UENUM()
enum EWeaponSlot
{
	Primary = 0,
	Secondary = 1,
	Melee = 2
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TGP_API UPlayerInventory : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventorySettings", Meta = (AllowPrivateAccess = true)) uint8 MaxUtilityAmount = 4;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventorySettings", Meta = (AllowPrivateAccess = true)) uint8 MaxConsumableAmount = 4;
	
	UPROPERTY() UItemContainer* UtilityContainer;
	UPROPERTY() UItemContainer* ConsumableContainer;
	UPROPERTY() UItemContainer* WeaponContainer;

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

	UPROPERTY(BlueprintAssignable, Category="WeaponChangedEvent")
	FOnWeaponChanged OnWeaponChangedEvent;
	
	UFUNCTION(BlueprintCallable) void PickUpWeapon(UWeaponItem* WeaponItem);
	UFUNCTION(BlueprintCallable) void ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate = false);
	UFUNCTION(BlueprintCallable) void DropWeapon();

	UFUNCTION(BlueprintCallable, BlueprintPure) class UWeaponItem* GetSelectedWeapon();
};
