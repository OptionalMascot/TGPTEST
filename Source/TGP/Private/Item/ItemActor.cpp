#include "Item/ItemActor.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::Initialize(UBaseItem* Item)
{
	if (Item == nullptr)
	{
		Destroy();
		return;
	}

	const UItemInfo* Info = Item->GetItemInfo();
	ItemMesh->SetStaticMesh(Info->ItemMesh);

	DefinedItem = Item;
}

