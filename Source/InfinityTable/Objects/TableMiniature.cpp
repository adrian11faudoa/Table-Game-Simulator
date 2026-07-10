#include "Objects/TableMiniature.h"
#include "Net/UnrealNetwork.h"

ATableMiniature::ATableMiniature()
{
    ObjectTypeID = TEXT("miniature");
}

void ATableMiniature::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableMiniature, MiniatureName);
    DOREPLIFETIME(ATableMiniature, HealthPoints);
    DOREPLIFETIME(ATableMiniature, MaxHealthPoints);
    DOREPLIFETIME(ATableMiniature, bOnGrid);
    DOREPLIFETIME(ATableMiniature, GridCell);
}

void ATableMiniature::Server_SetHealth_Implementation(int32 NewHP)
{
    HealthPoints = FMath::Clamp(NewHP, 0, MaxHealthPoints);
}

bool ATableMiniature::Server_SetHealth_Validate(int32 NewHP) { return true; }

void ATableMiniature::Server_MoveToCell_Implementation(FIntPoint Cell)
{
    GridCell = Cell;
    bOnGrid  = true;
}

bool ATableMiniature::Server_MoveToCell_Validate(FIntPoint Cell) { return true; }
