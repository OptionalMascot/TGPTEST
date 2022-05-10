#include "Inventory/PlayerInventory.h"
#include "Inventory/ItemContainer.h"
#include "Item/BaseItem.h"

UPlayerInventory::UPlayerInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInventory::BeginPlay()
{
	Super::BeginPlay();

	UtilityContainer = NewObject<UItemContainer>();
	UtilityContainer->Initialize(MaxUtilityAmount);

	ConsumableContainer = NewObject<UItemContainer>();
	ConsumableContainer->Initialize(MaxConsumableAmount);
}

void UPlayerInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerInventory::TryFindAndSelectValidUtility()
{
	SelectedUtilitySlot = UtilityContainer->FindFirstValidItem();
}

bool UPlayerInventory::AddUtility(UThrowableItem* ThrowableItem) const
{
	if (ThrowableItem)
		return UtilityContainer->AddItem(ThrowableItem);

	return false;
}

void UPlayerInventory::SelectUtility(uint8 Slot)
{
	if ((UThrowableItem*)UtilityContainer->GetItemAt(Slot) != nullptr)
		SelectedUtilitySlot = Slot;
}

void UPlayerInventory::PickUpWeapon(UWeaponItem* WeaponItem)
{
	
}

void UPlayerInventory::ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate)
{
	
}

void UPlayerInventory::DropWeapon()
{
	if (SelectedWeapon == EWeaponSlot::Primary)
	{
		
	}
}