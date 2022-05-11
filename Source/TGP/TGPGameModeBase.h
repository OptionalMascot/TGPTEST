#pragma once

#include "Item/BaseItem.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameInstance/BaseGameInstance.h"
#include "TGPGameModeBase.generated.h"

UCLASS()
class TGP_API ATGPGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATGPGameModeBase();
	
	template<class T>
	T* CreateItemByShortName(const FString& ItemShortName, const int Amount = 1);

	template<class T>
	T* CreateItemByUniqueId(int UniqueId, const int Amount = 1);
};

template <class T>
T* ATGPGameModeBase::CreateItemByShortName(const FString& ItemShortName, const int Amount)
{
	if (TIsDerivedFrom<T, UBaseItem>::IsDerived)
	{
		UItemInfo* Info = Cast<UBaseGameInstance>(GetGameInstance())->FindInfoShortName(ItemShortName);

		if (Info != nullptr)
		{
			T* NewItem = NewObject<T>();
			Cast<UBaseItem>(NewItem)->Init(Info, Amount);
		}
	}
	
	return nullptr;
}

template <class T>
T* ATGPGameModeBase::CreateItemByUniqueId(int UniqueId, const int Amount)
{
	UItemInfo* Info = Cast<UBaseGameInstance>(GetGameInstance())->FindInfoUniqueId(UniqueId);

	if (Info != nullptr)
	{
		T* NewItem = NewObject<T>();
		Cast<UBaseItem>(NewItem)->Init(Info, Amount);

		return NewItem;
	}
	
	return nullptr;
}