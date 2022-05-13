#include "Item/ItemActor.h"
#include "Item/BaseItem.h"
#include "Item/ItemInfo.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/UI/WeaponStatUIWidget.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = true;

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
	ItemPointLight->SetupAttachment(ItemSkeletalMesh);

	StatWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	StatWidget->SetupAttachment(RootComponent);
	StatWidget->SetWidgetClass(UWeaponStatUIWidget::StaticClass());
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	_playerController = nullptr;
}

void AItemActor::WidgetBillboard()
{
	if(!_playerController)
	{
		_playerController = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0);
	}
	else
	{
		FVector start = GetActorLocation();
		FVector target = _playerController->GetPawn()->GetActorLocation();
		
		FRotator rotator = UKismetMathLibrary::FindLookAtRotation(start, target);
		StatWidget->SetRelativeRotation(rotator);	
	}
}

void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	WidgetBillboard();
}

void AItemActor::LightColourSetup(const UWeaponInfo* info) const
{
	FColor color;
	switch(info->ItemRarity)
	{
	case ERarity::Common:
		color = FColor::White;
		break;
	case ERarity::Uncommon:
		color = FColor::Green;
		break;
	case ERarity::Rare:
		color = FColor::Blue;
		break;
	case ERarity::SuperRare:
		color = FColor::Purple;
		break;
	case ERarity::Legendary:
		color = FColor::Orange;
		break;
	default:
		color = FColor::White;
		break;
	}
	ItemPointLight->SetLightColor(color);
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
	{
		ItemSkeletalMesh->SetSkeletalMesh(WepInfo->WeaponSkeletalMesh);
		LightColourSetup(WepInfo);
	}
	else
		ItemMesh->SetStaticMesh(Info->ItemMesh);

	DefinedItem = Item;
}

