
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include "Inventory/ItemContainer.h"

UBaseItem::UBaseItem()
{
	
}

void UBaseItem::Init(UItemInfo* Info, int Amount)
{
	this->ItemInfo = Info;
	this->ItemAmount = Amount;
}

void UBaseItem::OnUse()
{
	ItemAmount -= 1;

	if (ItemAmount <= 0)
		OwningContainer->RemoveItem(this);
}

int UBaseItem::TryStack(UBaseItem* Item)
{
	if (Item->ItemInfo->UniqueId == ItemInfo->UniqueId)
	{
		ItemAmount += Item->GetAmount();

		if (ItemAmount >= ItemInfo->MaxStack)
		{
			const int Remaining = ItemAmount - ItemInfo->MaxStack;
			ItemAmount = ItemInfo->MaxStack;

			return Remaining;
		}

		return 0;
	}

	return Item->GetAmount();
}

void UGunItem::Init(UItemInfo* Info, int Amount)
{
	Super::Init(Info, Amount);

	const UGunInfo* GunInfo = Cast<UGunInfo>(Info);
	AmmoInClip = GunInfo->ClipSize;
	AmmoCount = GunInfo->DefaultAmmoReserve;
}
