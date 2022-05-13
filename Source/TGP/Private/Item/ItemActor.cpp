#include "Item/ItemActor.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetRelativeLocation(FVector::ZeroVector);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	ItemSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemSkeletalMesh"));
	ItemSkeletalMesh->SetupAttachment(RootComponent);
	ItemSkeletalMesh->SetSimulatePhysics(true);
	ItemSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	ItemSkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	ItemSkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	ItemPointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ItemLight"));
	ItemPointLight->SetupAttachment(RootComponent);
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
	const UWeaponInfo* WepInfo = Cast<UWeaponInfo>(Info);

	if (WepInfo != nullptr)
		ItemSkeletalMesh->SetSkeletalMesh(WepInfo->WeaponSkeletalMesh);
	else
		ItemMesh->SetStaticMesh(Info->ItemMesh);

	DefinedItem = Item;
}

