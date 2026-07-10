#include "ITGameState.h"
#include "Net/UnrealNetwork.h"

void AITGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(AITGameState, CurrentEnvironmentID);
    DOREPLIFETIME(AITGameState, bRPGModeEnabled);
    DOREPLIFETIME(AITGameState, bFogOfWarEnabled);
}
