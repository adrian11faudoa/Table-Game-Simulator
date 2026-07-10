#include "ITPlayerState.h"
#include "Net/UnrealNetwork.h"

void AITPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(AITPlayerState, Role);
    DOREPLIFETIME(AITPlayerState, PlayerColor);
    DOREPLIFETIME(AITPlayerState, Score);
}
