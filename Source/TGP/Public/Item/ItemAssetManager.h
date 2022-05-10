#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ItemAssetManager.generated.h"

UCLASS()
class TGP_API UItemAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UItemAssetManager();

	virtual void StartInitialLoading() override;

	static UItemAssetManager& Get();
};
