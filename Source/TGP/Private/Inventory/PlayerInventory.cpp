#include "Inventory/PlayerInventory.h"

#include "Engine/ActorChannel.h"
#include "Inventory/ItemContainer.h"
#include "Item/BaseItem.h"
#include "Item/ItemActor.h"
#include "Item/ItemInfo.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TGP/TGPGameModeBase.h"

UPlayerInventory::UPlayerInventory()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicated(true);
}

void UPlayerInventory::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		UtilityContainer = NewObject<UItemContainer>(GetOwner());
		UtilityContainer->Initialize(MaxUtilityAmount);

		ConsumableContainer = NewObject<UItemContainer>(GetOwner());
		ConsumableContainer->Initialize(MaxConsumableAmount);

		WeaponContainer = NewObject<UItemContainer>(GetOwner());
		WeaponContainer->Initialize(3);

		InitDefaultGuns();
	}

	SelectedWeapon = EWeaponSlot::Melee;

	//if (GetOwner()->HasAuthority())
	//	InitDefaultGuns();
	
   //ATGPGameModeBase* GameMode = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
   //
   //if (GameMode) // DEBUG
   //{
   //	bool b = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UGunItem>(72953608, 1));
   //	bool b2 = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UGunItem>(214248416, 1));
   //	bool b3 = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UGunItem>(137833872, 1));
   //}
}

void UPlayerInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerInventory::AddWeapon_Implementation(UWeaponItem* Item, int Slot)
{
	if (Item != nullptr)
	{
		UBaseItem* ItemAtSlot = WeaponContainer->GetItemAt(Slot);
	
		//if (ItemAtSlot)
		//DropWeapon(Slot);
	
		WeaponContainer->AddItem(Item, Slot);
		ItemAtSlot = WeaponContainer->GetItemAt(Slot);

		return;
	}
}

void UPlayerInventory::TryFindAndSelectValidUtility()
{
	SelectedUtilitySlot = UtilityContainer->FindFirstValidItem();
}

void UPlayerInventory::InitDefaultGuns()
{
	ATGPGameModeBase* GM = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	if (GM)
	{
		AddWeapon(GM->CreateItemByUniqueId<UGunItem>(72953608), 0);
		AddWeapon(GM->CreateItemByUniqueId<UGunItem>(214248416), 1);
		AddWeapon(GM->CreateItemByUniqueId<UGunItem>(137833872), 2);

		AddUtility(GM->CreateItemByUniqueId<UThrowableItem>(92876440, 3));
		AddUtility(GM->CreateItemByUniqueId<UThrowableItem>(111947304, 3));
	}
}

void UPlayerInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerInventory, UtilityContainer);
	DOREPLIFETIME(UPlayerInventory, ConsumableContainer);
	DOREPLIFETIME(UPlayerInventory, WeaponContainer);
}

bool UPlayerInventory::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bUpdate = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	bUpdate |= Channel->ReplicateSubobject(UtilityContainer, *Bunch, *RepFlags);
	bUpdate |= Channel->ReplicateSubobject(ConsumableContainer, *Bunch, *RepFlags);
	bUpdate |= Channel->ReplicateSubobject(WeaponContainer, *Bunch, *RepFlags);

	bUpdate |= UtilityContainer->ReplicateItems(Channel, Bunch, RepFlags);
	bUpdate |= ConsumableContainer->ReplicateItems(Channel, Bunch, RepFlags);
	bUpdate |= WeaponContainer->ReplicateItems(Channel, Bunch, RepFlags);
	
	return bUpdate;
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

UThrowableItem* UPlayerInventory::GetSelectedUtility()
{
	UThrowableItem* Item = Cast<UThrowableItem>(UtilityContainer->GetItemAt(SelectedUtilitySlot));

	if (Item == nullptr)
	{
		SelectedUtilitySlot = UtilityContainer->FindFirstValidItem();
		Item = Cast<UThrowableItem>(UtilityContainer->GetItemAt(SelectedUtilitySlot));
	}
	
	return Item;
}

void UPlayerInventory::ComponentLoadComplete()
{
	ChangeWeapon(EWeaponSlot::Primary, true);
}

bool UPlayerInventory::PickUpWeapon(UWeaponItem* WeaponItem)
{
	bool bPickedUp = false;
	
	if (WeaponContainer->GetItemAt(SelectedWeapon) == nullptr)
	{
		bPickedUp |= WeaponContainer->AddItem(WeaponItem);
		ChangeWeapon(SelectedWeapon, true);
	}
	
	for(int i = 0; i < 2; i++)
	{
		const UBaseItem* Item = WeaponContainer->GetItemAt(i);
		if (Item == nullptr)
		{
			WeaponContainer->AddItem(WeaponItem, i);
			return true;
		}
	}
	
	if (!bPickedUp && SelectedWeapon != EWeaponSlot::Melee)
	{
		const EWeaponSlot ActiveSlot = SelectedWeapon;
		
		DropWeapon(SelectedWeapon);
		SelectedWeapon = ActiveSlot;
		PickUpWeapon(WeaponItem);
		ChangeWeapon(SelectedWeapon, true);

		bPickedUp = true;
	}

	return bPickedUp;
}

void UPlayerInventory::ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate, bool bBroadcastChange)
{
	if ((Cast<UWeaponItem>(WeaponContainer->GetItemAt(Slot)) != nullptr && Slot != SelectedWeapon) || bForceUpdate)
	{
		SelectedWeapon = Slot;

		if (bBroadcastChange)
			OnWeaponChangedEvent.Broadcast(Cast<UWeaponItem>(WeaponContainer->GetItemAt(SelectedWeapon)));
	}
 }

void UPlayerInventory::DropWeapon(int Slot)
{
	if (Slot == -1)
		Slot = SelectedWeapon;
	
	if (Slot != EWeaponSlot::Melee)
	{
		UBaseItem* ItemToDrop = WeaponContainer->GetItemAt(Slot);

		if (ItemToDrop != nullptr)
		{
			//AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector() * 100.f), FRotator());
			//APlayerController* _playerController = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0);
			//
			//ItemActor->Initialize(ItemToDrop);
			//
			//FVector pos;
			//FRotator rot;
			//_playerController->GetPlayerViewPoint(pos, rot);
			//ItemActor->AddInitialThrowForce(rot.Vector(), 1000000.0f);
			
			WeaponContainer->RemoveItem(Slot);

			if (Slot == SelectedWeapon)
			{
				for	(int i = 0; i < 2; i ++)
				{
					if (WeaponContainer->GetItemAt(i) != nullptr)
					{
						ChangeWeapon((EWeaponSlot)i);
						return;
					}
				}
				
				ChangeWeapon(EWeaponSlot::Melee, true);
			}
		}
	}
}

bool UPlayerInventory::TryPickUpItem(UBaseItem* Item)
{
	if (UWeaponItem* Wep = Cast<UWeaponItem>(Item))
		return PickUpWeapon(Wep);

	return false;
}

UWeaponItem* UPlayerInventory::GetSelectedWeapon()
{
	return Cast<UWeaponItem>(WeaponContainer->GetItemAt(SelectedWeapon));
}

void UPlayerInventory::GenItems_Implementation()
{
	ATGPGameModeBase* GM = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	if (GM)
		WeaponContainer->AddItem(GM->CreateItemByUniqueId<UGunItem>(72953608, 1, GetOwner()));
}

void UPlayerInventory::PrintWeaponItems()
{
	for(uint8 i = 0; i < 3; i++)
	{
		auto Item = WeaponContainer->GetItemAt(i);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Item ? FString::FromInt(Item->GetItemId()) : "NULL");
	}
}

void UPlayerInventory::OnUseUtility()
{
	UtilityContainer->RemoveItem(SelectedUtilitySlot, 1);
}
