#include "Inventory/PlayerInventory.h"
#include "Inventory/ItemContainer.h"
#include "Item/BaseItem.h"

#include "Kismet/GameplayStatics.h"
#include "TGP/TGPGameModeBase.h"
    

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

	WeaponContainer = NewObject<UItemContainer>();
	WeaponContainer->Initialize(3);
	
    ATGPGameModeBase* GameMode = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
    
    if (GameMode)
    {
    	bool b = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UWeaponItem>(72953608, 1));
    	bool b2 = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UWeaponItem>(214248416, 1));
    	bool b3 = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UWeaponItem>(137833872, 1));
    }
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
	if (Cast<UThrowableItem>(UtilityContainer->GetItemAt(Slot)) != nullptr)
		SelectedUtilitySlot = Slot;
}

void UPlayerInventory::PickUpWeapon(UWeaponItem* WeaponItem)
{
	if (WeaponContainer->GetItemAt(SelectedWeapon) == nullptr)
		WeaponContainer->AddItem(WeaponItem);
	else if (const int Slot = WeaponContainer->FindFirstInValidItem() != -1)
		WeaponContainer->AddItem(WeaponItem, Slot);
}

void UPlayerInventory::ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate)
{
	if (Cast<UWeaponItem>(WeaponContainer->GetItemAt(Slot)) != nullptr)
	{
		SelectedWeapon = Slot;
		OnWeaponChangedEvent.Broadcast(Cast<UWeaponItem>(WeaponContainer->GetItemAt(SelectedWeapon)));
	}
 }

void UPlayerInventory::DropWeapon()
{
	if (SelectedWeapon == EWeaponSlot::Primary)
	{
		
	}
}

UWeaponItem* UPlayerInventory::GetSelectedWeapon()
{
	return Cast<UWeaponItem>(WeaponContainer->GetItemAt(SelectedWeapon));
}