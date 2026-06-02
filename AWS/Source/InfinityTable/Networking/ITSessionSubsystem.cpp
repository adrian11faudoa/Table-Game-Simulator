#include "ITSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

const FName UITSessionSubsystem::SESSION_NAME = TEXT("InfinityTableSession");

void UITSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS) return;
    SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;
    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UITSessionSubsystem::OnCreateSessionComplete);
    SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UITSessionSubsystem::OnFindSessionsComplete);
    SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UITSessionSubsystem::OnJoinSessionComplete);
}

void UITSessionSubsystem::HostSession(const FITSessionConfig& Config)
{
    if (!SessionInterface.IsValid()) return;
    FOnlineSessionSettings S;
    S.NumPublicConnections   = Config.MaxPlayers;
    S.bIsLANMatch            = Config.bLAN;
    S.bUsesPresence          = true;
    S.bShouldAdvertise       = !Config.bPrivate;
    S.bAllowJoinInProgress   = true;
    S.bUseLobbiesIfAvailable = true;
    S.Set(FName("GAME_NAME"), Config.GameName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    SessionInterface->CreateSession(0, SESSION_NAME, S);
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

void UITSessionSubsystem::JoinSession(int32 Idx)
{
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;
    if (!SessionSearch->SearchResults.IsValidIndex(Idx)) return;
    SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Idx]);
}

void UITSessionSubsystem::DestroySession()
{ if (SessionInterface.IsValid()) SessionInterface->DestroySession(SESSION_NAME); }

bool UITSessionSubsystem::IsInSession() const
{ return SessionInterface.IsValid() && SessionInterface->GetNamedSession(SESSION_NAME) != nullptr; }

void UITSessionSubsystem::OnCreateSessionComplete(FName Name, bool bSuccess)
{
    OnSessionCreated.Broadcast(bSuccess);
    if (bSuccess)
        if (UWorld* W = GetGameInstance()->GetWorld())
            W->ServerTravel(TEXT("/Game/Maps/TableMap?listen"), true);
}

void UITSessionSubsystem::OnFindSessionsComplete(bool bSuccess)
{
    TArray<FITSessionResult> Results;
    if (bSuccess && SessionSearch.IsValid())
        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
        {
            const auto& SR = SessionSearch->SearchResults[i];
            FITSessionResult R;
            SR.Session.SessionSettings.Get(FName("GAME_NAME"), R.GameName);
            R.CurrentPlayers    = SR.Session.SessionSettings.NumPublicConnections - SR.Session.NumOpenPublicConnections;
            R.MaxPlayers        = SR.Session.SessionSettings.NumPublicConnections;
            R.Ping              = SR.PingInMs;
            R.SearchResultIndex = i;
            Results.Add(R);
        }
    OnSessionsFound.Broadcast(Results);
}

void UITSessionSubsystem::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result)
{
    bool bOK = (Result == EOnJoinSessionCompleteResult::Success);
    OnSessionJoined.Broadcast(bOK);
    if (bOK && SessionInterface.IsValid())
    {
        FString Addr;
        SessionInterface->GetResolvedConnectString(SESSION_NAME, Addr);
        if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
            PC->ClientTravel(Addr, TRAVEL_Absolute);
    }
}
