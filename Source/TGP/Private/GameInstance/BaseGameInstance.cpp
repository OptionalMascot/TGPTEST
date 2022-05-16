// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/BaseGameInstance.h"
#include "Engine/AssetManager.h"
#include "Item/ItemInfo.h"
#include "steam/steam_api.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UBaseGameInstance::UBaseGameInstance() : CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnCreateSessionComplete)),
	SessionInviteReceivedDelegate(FOnSessionInviteReceivedDelegate::CreateUObject(this, &UBaseGameInstance::OnSessionInviteReceived)),
	SessionInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteAccepted)),
	OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnJoinSessionComplete))
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

	//if (!SteamAPI_Init())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Failed To Init Steamworks API"));
	//	return;
	//}

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("No Valid Online Session Found"));
		return;
	}

	SessionInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegate);
	SessionInviteReceivedDelegateHandle = SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(SessionInviteReceivedDelegate);
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
		if (ItemInfos[i]->UniqueId == UniqueId)
			return ItemInfos[i];

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

void UBaseGameInstance::CreateOnlineSession()
{
	const IOnlineSessionPtr Session = Online::GetSessionInterface();

	if (!Session.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed To Get Valid Online Session Interface"));
		OnCreateSessionCompleteEvent.Broadcast(NAME_GameSession, false);
		return;
	}

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings->bIsDedicated = false;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bShouldAdvertise = false;

	SessionSettings->Set(SETTING_MAPNAME, FString("Lobby"), EOnlineDataAdvertisementType::ViaOnlineService);
	CreateSessionCompleteDelegateHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!Session->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
	{
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnCreateSessionCompleteEvent.Broadcast(NAME_GameSession, false);
	}
}

bool UBaseGameInstance::SendSessionInvite(APlayerController* PlayerController, const FUniqueNetId& FriendId)
{
	if (PlayerController && FriendId.IsValid())
	{
		IOnlineSessionPtr Session = Online::GetSessionInterface();
		ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);
		TSharedPtr<const FUniqueNetId> Id = Online::GetIdentityInterface()->GetUniquePlayerId(0);

		if (Session && Player)
			return Session->SendSessionInviteToFriend(Player->GetControllerId(), NAME_GameSession, FriendId);
	}

	return false;
}

void UBaseGameInstance::OnSessionInviteAccepted(const bool bSuccessful, int32 LocalPlayer, TSharedPtr<const FUniqueNetId> InviterId, const FOnlineSessionSearchResult& SessionToJoin)
{
	if (bSuccessful)
	{
		if (SessionToJoin.IsValid())
		{
			TSharedPtr<const FUniqueNetId> Id = Online::GetIdentityInterface()->GetUniquePlayerId(LocalPlayer);

			UE_LOG(LogTemp, Display, TEXT("Invite Accepted"));

			if (Id.IsValid())
				JoinFriendSession(Id, NAME_GameSession, SessionToJoin);
		}
	}
}

void UBaseGameInstance::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& SenderId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "Invite Received");
}

void UBaseGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Session = OnlineSub->GetSessionInterface();

		if (Session.IsValid())
		{
			Session->ClearOnCreateSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
			APlayerController* PlayerController = GetFirstLocalPlayerController();
			FString Ip;
			
			if (PlayerController && Session->GetResolvedConnectString(SessionName, Ip))
				PlayerController->ClientTravel(Ip, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UBaseGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccessful)
{
	const IOnlineSessionPtr Session = Online::GetSessionInterface();

	OnCreateSessionCompleteEvent.Broadcast(SessionName, bSuccessful);

	if (Session)
		Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
}

bool UBaseGameInstance::JoinFriendSession(TSharedPtr<const FUniqueNetId> LocalUserId, FName SessionName, const FOnlineSessionSearchResult& Result)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		IOnlineSessionPtr Session = OnlineSub->GetSessionInterface();

		if (Session.IsValid() && LocalUserId.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
			return Session->JoinSession(*LocalUserId, SessionName, Result);
		}
	}

	return false;
}

UTexture2D* UBaseGameInstance::GetFriendAvatar(TSharedRef<const FUniqueNetId> FriendUniqueNetId) const
{
	if (FriendUniqueNetId->IsValid() && FriendUniqueNetId->GetType() == STEAM_SUBSYSTEM)
	{
		if (SteamAPI_Init())
		{
			uint64 SteamID = *((uint64*)FriendUniqueNetId->GetBytes());

			int Picture = SteamFriends()->GetMediumFriendAvatar(SteamID);

			if (Picture == -1)
				return nullptr;

			uint32 Width = 0;
			uint32 Height = 0;

			SteamUtils()->GetImageSize(Picture, &Width, &Height);

			if (Width > 0 && Height > 0)
			{
				uint8 *oAvatarRGBA = new uint8[Width * Height * 4];
				
				SteamUtils()->GetImageRGBA(Picture, oAvatarRGBA, 4 * Height * Width * sizeof(char));

				UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
				uint8* MipData = (uint8*)Avatar->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(MipData, oAvatarRGBA, Height * Width * 4);
				Avatar->PlatformData->Mips[0].BulkData.Unlock();
				
				delete[] oAvatarRGBA;
				
				Avatar->PlatformData->SetNumSlices(1);
				Avatar->NeverStream = true;

				Avatar->UpdateResource();
				
				return Avatar;
			}
		}
	}
	
	return nullptr;
}