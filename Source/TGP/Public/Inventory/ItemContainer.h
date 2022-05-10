#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemContainer.generated.h"

class UBaseItem;

UCLASS()
class TGP_API UItemContainer : public UObject
{
	GENERATED_BODY()

	TArray<UBaseItem*> Items;

public:
	UItemContainer();

	void Initialize(int ContainerSize);

	UBaseItem* GetItemAt(int Slot) { return InBounds(Slot) ? Items[Slot] : nullptr; }
	int FindFirstValidItem();
	
	bool AddItem(UBaseItem* Item);
	bool AddItem(UBaseItem* Item, int Slot);
	
	bool RemoveItem(UBaseItem* Item);
	bool RemoveItem(int Index, int Amount);
	bool RemoveItem(int Slot);

	bool InBounds(int Slot) const { return Slot >= 0 && Slot < Items.Num(); }
};
