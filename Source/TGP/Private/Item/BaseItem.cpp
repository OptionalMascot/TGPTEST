
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include "Inventory/ItemContainer.h"

UBaseItem::UBaseItem()
{
	
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
