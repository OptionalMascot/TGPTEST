#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseItem.generated.h"

class UItemInfo;
class UItemContainer;

UCLASS(BlueprintType)
class TGP_API UBaseItem : public UObject
{
	GENERATED_BODY()

protected:
	int ItemAmount = 1;
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn)) UItemInfo* ItemInfo;
	UPROPERTY() UItemContainer* OwningContainer;
	
public:
	UBaseItem();

	UFUNCTION(BlueprintCallable, BlueprintPure) UItemInfo* GetItemInfo() const { return ItemInfo; }

	virtual void Init(UItemInfo* Info, int Amount);
	void SetOwningContainer(UItemContainer* Container) { OwningContainer = Container; }
	
	int GetAmount() const { return ItemAmount; }
	void SetAmount(int Amount) { ItemAmount = Amount; }

	void OnUse();
	int TryStack(UBaseItem* Item);
};

UCLASS()
class TGP_API UWeaponItem : public UBaseItem
{
	GENERATED_BODY()

public:
};

UCLASS()
class TGP_API UGunItem : public UWeaponItem
{
	GENERATED_BODY()

	int AmmoCount = 120;
	int AmmoInClip;
	
public:
	virtual void Init(UItemInfo* Info, int Amount) override;
	
	int GetAmmoInClip() const { return AmmoInClip; }
	void SetAmmoInClip(int NewAmmo) { AmmoInClip = NewAmmo; }
	
	int GetAmmoCount() const { return AmmoCount; }
	void SetAmmoCount(int NewAmmoCount) { AmmoCount = NewAmmoCount; }
};


UCLASS()
class TGP_API UThrowableItem : public UWeaponItem
{
	GENERATED_BODY()

public:
};