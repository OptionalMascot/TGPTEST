#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BaseGameInstance.generated.h"

class UItemInfo;

UCLASS()
class TGP_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    
    FOnSessionInviteReceivedDelegate SessionInviteReceivedDelegate;
    FDelegateHandle SessionInviteReceivedDelegateHandle;
    
    FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;
    FDelegateHandle SessionInviteAcceptedDelegateHandle;
    
    FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	TArray<UItemInfo*> ItemInfos;

	void OnLoadedInfos();
	void LoadInfos();
	
	void OnSessionInviteAccepted(const bool bSuccessful, int32 LocalPlayer, TSharedPtr<const FUniqueNetId> InviterId, const FOnlineSessionSearchResult& SessionToJoin);
	void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& SenderId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnCreateSessionComplete(FName SessionName, bool bSuccessful);

	bool JoinFriendSession(TSharedPtr<const FUniqueNetId> LocalUserId, FName SessionName, const FOnlineSessionSearchResult& Result);
	
public:
	UBaseGameInstance();
	
	virtual void Init() override;
	virtual void Shutdown() override;

	UItemInfo* FindInfoShortName(const FString& ItemShortName) const;
	UItemInfo* FindInfoUniqueId(int UniqueId) const;

	int32 GetRandomItemIdOfCategory(enum EItemCategory ItemCategory);

	void CreateOnlineSession();
	bool SendSessionInvite(APlayerController* PlayerController, const FUniqueNetId& FriendId);

	FOnCreateSessionComplete OnCreateSessionCompleteEvent;
	UTexture2D* GetFriendAvatar(TSharedRef<const FUniqueNetId> FriendUniqueNetId) const;
};
