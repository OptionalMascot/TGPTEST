// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemInfo.h"
#include "Item/BaseItem.h"

void UItemInfo::GenerateStats()
{
	ItemShortName = FString::Printf(TEXT("%s.%s"), *UEnum::GetValueAsString(ItemCategory), *ItemName);
}

UItemInfo::UItemInfo()
{
	FMath::RandInit(FDateTime::Now().GetMillisecond() + FDateTime::Now().GetSecond());
	UniqueId = FMath::RandRange(78000, INT32_MAX);
	
	ItemClass = UBaseItem::StaticClass();
}

UGunInfo::UGunInfo()
{
	ItemClass = UGunItem::StaticClass();
	ItemCategory = EItemCategory::Weapon;
}

void UGunInfo::GenerateStats()
{
	Super::GenerateStats();

	ItemShortName = FString::Printf(TEXT("%s.%s.%s"), *UEnum::GetValueAsString(ItemCategory), *FString("Gun"), *ItemName);
}

UThrowableInfo::UThrowableInfo()
{
	ItemClass = UThrowableItem::StaticClass();
	ItemCategory = EItemCategory::Weapon;
}

void UThrowableInfo::GenerateStats()
{
	Super::GenerateStats();
	ItemShortName = FString::Printf(TEXT("%s.%s.%s"), *UEnum::GetValueAsString(ItemCategory), *FString("Throwable"), *ItemName);
}
