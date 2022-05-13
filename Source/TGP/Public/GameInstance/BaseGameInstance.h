#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

class UItemInfo;

UCLASS()
class TGP_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

	TArray<UItemInfo*> ItemInfos;

	void OnLoadedInfos();
	void LoadInfos();
	
public:
	UBaseGameInstance();
	
	virtual void Init() override;
	virtual void Shutdown() override;

	UItemInfo* FindInfoShortName(const FString& ItemShortName) const;
	UItemInfo* FindInfoUniqueId(int UniqueId) const;

	int32 GetRandomItemIdOfCategory(enum EItemCategory ItemCategory);
};
