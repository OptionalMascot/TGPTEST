#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemActor.h"
#include "PlayerInventory.generated.h"

class UItemContainer;
class UWeaponItem;
class UThrowableItem;
class UBaseItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, UWeaponItem*, WeaponItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAddWeapon);

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventorySettings", Meta = (AllowPrivateAccess = true)) TSubclassOf<AItemActor> ItemActorClass;
	
	UPROPERTY() UItemContainer* UtilityContainer;
	UPROPERTY() UItemContainer* ConsumableContainer;
	UPROPERTY() UItemContainer* WeaponContainer;

	EWeaponSlot SelectedWeapon;
	uint8 SelectedUtilitySlot = 0;

	void TryFindAndSelectValidUtility();

	UFUNCTION(Server, Reliable) void InitDefaultGuns();
	void InitDefaultGuns_Implementation();
	
protected:
	virtual void BeginPlay() override;

public:
	UPlayerInventory();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(NetMulticast, Reliable) void AddWeapon(UWeaponItem* Item, int Slot);
	void AddWeapon_Implementation(UWeaponItem* Item, int Slot);
	
	bool AddUtility(UThrowableItem* ThrowableItem) const;
	void SelectUtility(uint8 Slot);
	UThrowableItem* GetSelectedUtility();

	void ComponentLoadComplete();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category="WeaponChangedEvent")
	FOnWeaponChanged OnWeaponChangedEvent;

	UPROPERTY(BlueprintAssignable, Category="WeaponChangedEvent")
	FOnAddWeapon OnTest;
	
	bool PickUpWeapon(UWeaponItem* WeaponItem);
	UFUNCTION(BlueprintCallable) void DropWeapon(int Slot = -1);
	
	UFUNCTION(BlueprintCallable) void ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate = false);
	UFUNCTION(BlueprintCallable) bool TryPickUpItem(UBaseItem* Item);
	UFUNCTION(BlueprintCallable, BlueprintPure) class UWeaponItem* GetSelectedWeapon();

	void OnUseUtility();
};
