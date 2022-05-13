// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/BaseGameInstance.h"
#include "Engine/AssetManager.h"
#include "Item/ItemInfo.h"

UBaseGameInstance::UBaseGameInstance()
{
	
}

void UBaseGameInstance::OnLoadedInfos()
{
	const UAssetManager& AssetManager = UAssetManager::Get();

	TArray<UObject*> LoadedInfos;
	AssetManager.GetPrimaryAssetObjectList(FPrimaryAssetType("ItemInfo"), LoadedInfos);

	ItemInfos.Reserve(LoadedInfos.Num());

	for (UObject* Obj : LoadedInfos)
		ItemInfos.Add(Cast<UItemInfo>(Obj));
}

void UBaseGameInstance::LoadInfos()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	if (AssetManager.IsValid())
	{
		TArray<FPrimaryAssetId> PrimaryIds;
		
		AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("ItemInfo"), PrimaryIds);
		AssetManager.LoadPrimaryAssets(PrimaryIds, TArray<FName>(), FStreamableDelegate::CreateUObject(this, &UBaseGameInstance::OnLoadedInfos));
		
		ItemInfos.Reserve(PrimaryIds.Num());

		UE_LOG(LogTemp, Warning, TEXT("LOADED %d"), PrimaryIds.Num());
		
		for (FPrimaryAssetId Id : PrimaryIds)
			ItemInfos.Add(Cast<UItemInfo>(AssetManager.GetPrimaryAssetPath(Id).TryLoad()));
	}
}

void UBaseGameInstance::Init()
{
	Super::Init();

	LoadInfos();
}

void UBaseGameInstance::Shutdown()
{
	Super::Shutdown();

	UAssetManager& AssetManager = UAssetManager::Get();

	if (AssetManager.IsValid())
	{
		TArray<FPrimaryAssetId> PrimaryIds;
		
		AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("ItemInfo"), PrimaryIds);
		AssetManager.UnloadPrimaryAssets(PrimaryIds);
	}
}

UItemInfo* UBaseGameInstance::FindInfoShortName(const FString& ItemShortName) const
{
	for (UItemInfo* Info : ItemInfos)
		if (Info->ItemShortName == ItemShortName)
			return Info;

	return nullptr;
}

UItemInfo* UBaseGameInstance::FindInfoUniqueId(int UniqueId) const
{
	for (int i = 0; i < ItemInfos.Num(); i++)
	{
		if (ItemInfos[i]->UniqueId == UniqueId)
		{
			return ItemInfos[i];
		}
	}

	return nullptr;
}

int32 UBaseGameInstance::GetRandomItemIdOfCategory(EItemCategory ItemCategory)
{
	TArray<int> FoundIndexes;

	for (int i = 0; i < ItemInfos.Num(); i++)
		if (ItemInfos[i]->ItemCategory == ItemCategory)
			FoundIndexes.Add(i);

	if (FoundIndexes.Num() > 0)
		return ItemInfos[FoundIndexes[FMath::RandRange(0, FoundIndexes.Num() - 1)]]->UniqueId;
	
	return -1;
}