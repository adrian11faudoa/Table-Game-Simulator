#include "Objects/TableToken.h"
#include "Net/UnrealNetwork.h"

ATableToken::ATableToken()
{
    ObjectTypeID = TEXT("token");
}

void ATableToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableToken, TokenLabel);
    DOREPLIFETIME(ATableToken, CounterValue);
    DOREPLIFETIME(ATableToken, bFaceUp);
}

void ATableToken::Server_SetCounter_Implementation(int32 Value)
{
    CounterValue = Value;
}

bool ATableToken::Server_SetCounter_Validate(int32 Value) { return true; }

void ATableToken::Server_FlipToken_Implementation()
{
    bFaceUp = !bFaceUp;
    Server_Flip();
}

bool ATableToken::Server_FlipToken_Validate() { return true; }
