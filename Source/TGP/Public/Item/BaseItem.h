#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseItem.generated.h"

class UItemInfo;
class UItemContainer;

UCLASS()
class TGP_API UBaseItem : public UObject
{
	GENERATED_BODY()

protected:
	int ItemAmount = 1;
	UPROPERTY() UItemInfo* ItemInfo;
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

public:
};


UCLASS()
class TGP_API UThrowableItem : public UWeaponItem
{
	GENERATED_BODY()

public:
};