// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemAssetManager.h"

UItemAssetManager::UItemAssetManager()
{
	
}

void UItemAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
}

UItemAssetManager& UItemAssetManager::Get()
{
	return *Cast<UItemAssetManager>(GEngine->AssetManager);
}
