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
    
    if (GameMode) // DEBUG
    {
    	bool b = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UGunItem>(72953608, 1));
    	bool b2 = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UGunItem>(214248416, 1));
    	bool b3 = WeaponContainer->AddItem(GameMode->CreateItemByUniqueId<UGunItem>(137833872, 1));
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
		
		DropWeapon();
		SelectedWeapon = ActiveSlot;
		PickUpWeapon(WeaponItem);
		ChangeWeapon(SelectedWeapon, true);

		bPickedUp = true;
	}

	return bPickedUp;
}


void UPlayerInventory::ChangeWeapon(EWeaponSlot Slot, bool bForceUpdate)
{
	if ((Cast<UWeaponItem>(WeaponContainer->GetItemAt(Slot)) != nullptr && Slot != SelectedWeapon) || bForceUpdate)
	{
		SelectedWeapon = Slot;
		OnWeaponChangedEvent.Broadcast(Cast<UWeaponItem>(WeaponContainer->GetItemAt(SelectedWeapon)));
	}
 }

void UPlayerInventory::DropWeapon()
{
	if (SelectedWeapon != EWeaponSlot::Melee)
	{
		UBaseItem* ItemToDrop = WeaponContainer->GetItemAt(SelectedWeapon);

		if (ItemToDrop != nullptr)
		{
			AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector() * 100.f), FRotator());
			ItemActor->Initialize(ItemToDrop);

			USkeletalMeshComponent* mesh = ItemActor->GetSkeletalMesh();
			APlayerController* _playerController = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0);
			FVector pos;
			FRotator rot;
			_playerController->GetPlayerViewPoint(pos, rot);
			FVector force = rot.Vector() * 1000000.0f;
			mesh->AddForce(force);
			FVector randSpin = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f));
			mesh->AddTorque(randSpin * 1000000.0f);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::SanitizeFloat(randSpin.X) + "/" + FString::SanitizeFloat(randSpin.Y) + "/" + FString::SanitizeFloat(randSpin.Z));
			
			WeaponContainer->RemoveItem(SelectedWeapon);

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
