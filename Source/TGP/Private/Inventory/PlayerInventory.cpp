#include "Inventory/PlayerInventory.h"
#include "Inventory/ItemContainer.h"
#include "Item/BaseItem.h"
#include "Item/ItemActor.h"
#include "Item/ItemInfo.h"

#include "Kismet/GameplayStatics.h"
#include "TGP/TGPGameModeBase.h"

UPlayerInventory::UPlayerInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInventory::InitDefaultGuns_Implementation()
{
	ATGPGameModeBase* GM = Cast<ATGPGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	if (GM)
	{
		UGunItem* Item = GM->CreateItemByUniqueId<UGunItem>(72953608);

		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, Item != nullptr ? "VALIDDDDDDDDDDD" : "NOT VALSIE");
		
		AddWeapon(Item, 0);
		AddWeapon(GM->CreateItemByUniqueId<UGunItem>(214248416), 1);
		AddWeapon(GM->CreateItemByUniqueId<UGunItem>(137833872), 2);

		AddUtility(GM->CreateItemByUniqueId<UThrowableItem>(92876440, 3));
		AddUtility(GM->CreateItemByUniqueId<UThrowableItem>(111947304, 3));

		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, "NONONONONONO");
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

	if (GetOwner()->HasAuthority())
		InitDefaultGuns();
	
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

		if (GetWorld()->IsServer())
		{
			UE_LOG(LogTemp, Warning, TEXT("SERVER:   %s"), *ItemAtSlot->GetItemInfo()->ItemName);
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, "ITEM IS VALIIIIDDDD!!!!!!!!!!!");
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CLIENT :   %s     %d"), *ItemAtSlot->GetItemInfo()->ItemName, Slot);
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, "NONONONNONONNONN!!!!!!!!");
		}
		
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, "ITEM IS NULL");

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, "FAILED");
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

UThrowableItem* UPlayerInventory::GetSelectedUtility()
{
	UThrowableItem* Item = Cast<UThrowableItem>(UtilityContainer->GetItemAt(SelectedUtilitySlot));

	if (Item == nullptr)
	{
		SelectedUtilitySlot = UtilityContainer->FindFirstValidItem();
		Item = Cast<UThrowableItem>(UtilityContainer->GetItemAt(SelectebdUtilitySlot));
	}
	
	return Item;
}

void UPlayerInventory::ComponentLoadComplete()
{
	ChangeWeapon(EWeaponSlot::Primary, true);
}

void UPlayerInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UPlayerInventory::PickUpWeapon(UWeaponItem* WeaponItem)
{
	bool bPickedUp = false;
	
	if (WeaponContainer->GetItemAt(SelectedWeapon) == nullptr)
	{
		bPickedUp |= WeaponContainer->AddItem(WeaponItem);
		ChangeWeapon(SelectedWeapon, true);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "1");
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


void UPlayerInventory::ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("%d"), (int)Slot));
	
	if ((Cast<UWeaponItem>(WeaponContainer->GetItemAt(Slot)) != nullptr && Slot != SelectedWeapon) || bForceUpdate)
	{
		
		if (Cast<UWeaponItem>(WeaponContainer->GetItemAt(Slot)) != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, *(Cast<UWeaponItem>(WeaponContainer->GetItemAt(Slot))->GetItemInfo()->ItemName));
		}
		
		SelectedWeapon = Slot;
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
			AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector() * 100.f), FRotator());
			ItemActor->Initialize(ItemToDrop);
			APlayerController* _playerController = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0);
			FVector pos;
			FRotator rot;
			_playerController->GetPlayerViewPoint(pos, rot);
			ItemActor->AddInitialThrowForce(rot.Vector(), 1000000.0f);
			
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

void UPlayerInventory::OnUseUtility()
{
	UtilityContainer->RemoveItem(SelectedUtilitySlot, 1);
}
