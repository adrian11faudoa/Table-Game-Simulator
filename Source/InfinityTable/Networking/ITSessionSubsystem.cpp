#include "ITSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

const FName UITSessionSubsystem::SESSION_NAME = TEXT("InfinityTableSession");

void UITSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS) { UE_LOG(LogTemp, Warning, TEXT("No OSS found")); return; }

    SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    OnCreateHandle  = SessionInterface->OnCreateSessionCompleteDelegates .AddUObject(this, &UITSessionSubsystem::OnCreateSessionComplete);
    OnFindHandle    = SessionInterface->OnFindSessionsCompleteDelegates  .AddUObject(this, &UITSessionSubsystem::OnFindSessionsComplete);
    OnJoinHandle    = SessionInterface->OnJoinSessionCompleteDelegates   .AddUObject(this, &UITSessionSubsystem::OnJoinSessionComplete);
    OnDestroyHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UITSessionSubsystem::OnDestroySessionComplete);
}

void UITSessionSubsystem::HostSession(const FITSessionConfig& Config)
{
    if (!SessionInterface.IsValid()) return;

    // Destroy existing session first
    if (IsInSession()) SessionInterface->DestroySession(SESSION_NAME);

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections    = Config.MaxPlayers;
    Settings.NumPrivateConnections   = 0;
    Settings.bIsLANMatch             = Config.bLAN;
    Settings.bUsesPresence           = true;
    Settings.bAllowJoinInProgress    = Config.bAllowJoinInProgress;
    Settings.bShouldAdvertise        = !Config.bPrivate;
    Settings.bUseLobbiesIfAvailable  = true;
    Settings.bAllowInvites           = true;

    Settings.Set(FName("GAME_NAME"),   Config.GameName,    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    Settings.Set(FName("ENVIRONMENT"), Config.EnvironmentID, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    Settings.Set(FName("HAS_PASSWORD"), !Config.Password.IsEmpty(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->CreateSession(0, SESSION_NAME, Settings);
}

void UITSessionSubsystem::FindSessions(int32 MaxResults)
{
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = MaxResults;
    SessionSearch->bIsLanQuery      = false;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UITSessionSubsystem::JoinSession(int32 SearchResultIndex)
{
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;
    if (!SessionSearch->SearchResults.IsValidIndex(SearchResultIndex)) return;
    SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[SearchResultIndex]);
}

void UITSessionSubsystem::DestroySession()
{
    if (SessionInterface.IsValid()) SessionInterface->DestroySession(SESSION_NAME);
}

bool UITSessionSubsystem::IsInSession() const
{
    if (!SessionInterface.IsValid()) return false;
    return SessionInterface->GetNamedSession(SESSION_NAME) != nullptr;
}

void UITSessionSubsystem::OnCreateSessionComplete(FName Name, bool bSuccess)
{
    OnSessionCreated.Broadcast(bSuccess);
    if (bSuccess)
    {
        UWorld* World = GetGameInstance()->GetWorld();
        if (World) World->ServerTravel(TEXT("/Game/Maps/TableMap?listen"), true);
    }
}

void UITSessionSubsystem::OnFindSessionsComplete(bool bSuccess)
{
    TArray<FITSessionResult> Results;
    if (bSuccess && SessionSearch.IsValid())
    {
        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
        {
            const FOnlineSessionSearchResult& SR = SessionSearch->SearchResults[i];
            FITSessionResult R;
            SR.Session.SessionSettings.Get(FName("GAME_NAME"),   R.GameName);
            SR.Session.SessionSettings.Get(FName("ENVIRONMENT"), R.EnvironmentID);
            SR.Session.SessionSettings.Get(FName("HAS_PASSWORD"),R.bHasPassword);
            R.CurrentPlayers    = SR.Session.SessionSettings.NumPublicConnections - SR.Session.NumOpenPublicConnections;
            R.MaxPlayers        = SR.Session.SessionSettings.NumPublicConnections;
            R.Ping              = SR.PingInMs;
            R.SearchResultIndex = i;
            Results.Add(R);
        }
    }
    OnSessionsFound.Broadcast(Results);
}

void UITSessionSubsystem::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result)
{
    bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);
    OnSessionJoined.Broadcast(bSuccess);

    if (bSuccess && SessionInterface.IsValid())
    {
        FString ConnectAddr;
        SessionInterface->GetResolvedConnectString(SESSION_NAME, ConnectAddr);
        APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
        if (PC) PC->ClientTravel(ConnectAddr, TRAVEL_Absolute);
    }
}

void UITSessionSubsystem::OnDestroySessionComplete(FName Name, bool bSuccess)
{
    OnSessionDestroyed.Broadcast(bSuccess);
}
