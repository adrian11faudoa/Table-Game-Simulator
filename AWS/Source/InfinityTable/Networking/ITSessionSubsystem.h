#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ITSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FITSessionConfig
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) FString GameName;
    UPROPERTY(BlueprintReadWrite) FString EnvironmentID = TEXT("FantasyTavern");
    UPROPERTY(BlueprintReadWrite) int32   MaxPlayers    = 6;
    UPROPERTY(BlueprintReadWrite) bool    bLAN          = false;
    UPROPERTY(BlueprintReadWrite) bool    bPrivate      = false;
    UPROPERTY(BlueprintReadWrite) FString Password;
};

USTRUCT(BlueprintType)
struct FITSessionResult
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString GameName;
    UPROPERTY(BlueprintReadOnly) int32   CurrentPlayers = 0;
    UPROPERTY(BlueprintReadOnly) int32   MaxPlayers     = 6;
    UPROPERTY(BlueprintReadOnly) int32   Ping           = 0;
    int32 SearchResultIndex = -1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFound, const TArray<FITSessionResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool, bSuccess);

UCLASS()
class INFINITYTABLE_API UITSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UFUNCTION(BlueprintCallable) void HostSession(const FITSessionConfig& Config);
    UFUNCTION(BlueprintCallable) void FindSessions(int32 MaxResults = 50);
    UFUNCTION(BlueprintCallable) void JoinSession(int32 SearchResultIndex);
    UFUNCTION(BlueprintCallable) void DestroySession();
    UFUNCTION(BlueprintCallable) bool IsInSession() const;
    UPROPERTY(BlueprintAssignable) FOnSessionCreated OnSessionCreated;
    UPROPERTY(BlueprintAssignable) FOnSessionsFound  OnSessionsFound;
    UPROPERTY(BlueprintAssignable) FOnSessionJoined  OnSessionJoined;
    static const FName SESSION_NAME;
private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;
    void OnCreateSessionComplete(FName Name, bool bSuccess);
    void OnFindSessionsComplete(bool bSuccess);
    void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result);
};
