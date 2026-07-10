#include "UI/RadialMenuWidget.h"
#include "Objects/TableObject.h"
#include "Objects/TableCard.h"
#include "Objects/TableDeck.h"
#include "Objects/TableDice.h"

void URadialMenuWidget::OpenForObject(ATableObject* Target)
{
    CurrentTarget = Target;
    if (!Target) return;

    SetVisibility(ESlateVisibility::Visible);
    OnOpened(BuildOptionsForTarget(Target));
}

void URadialMenuWidget::Close()
{
    CurrentTarget = nullptr;
    SetVisibility(ESlateVisibility::Collapsed);
    OnClosed();
}

TArray<FRadialMenuOption> URadialMenuWidget::BuildOptionsForTarget(ATableObject* Target) const
{
    TArray<FRadialMenuOption> Options;

    auto Add = [&](const FString& ID, const FString& Label)
    {
        FRadialMenuOption Opt;
        Opt.ActionID = ID;
        Opt.Label    = Label;
        Options.Add(Opt);
    };

    Add(TEXT("flip"),     TEXT("Flip"));
    Add(TEXT("rotate"),   TEXT("Rotate"));
    Add(TEXT("lock"),     Target->bPhysicsLocked ? TEXT("Unlock") : TEXT("Lock"));
    Add(TEXT("duplicate"),TEXT("Duplicate"));
    // "Group" (combining multiple objects into a new group) requires a
    // multi-object selection mechanism that doesn't exist yet — there's no
    // way to select more than one ATableObject at a time on the table, so
    // the radial menu (which always targets exactly one object) can only
    // meaningfully offer "Ungroup" here. The full Server_SetGroup/
    // MoveGroupBy/LockGroup/DestroyGroup API on ATableObject is still
    // available to Lua mods, which can assign GroupIDs deliberately without
    // needing a UI selection step — see Docs/Modding_API.md.
    if (!Target->GroupID.IsEmpty())
        Add(TEXT("ungroup"), TEXT("Ungroup"));
    Add(TEXT("delete"),   TEXT("Delete"));

    if (Target->IsA(ATableDice::StaticClass()))
        Add(TEXT("roll"), TEXT("Roll"));

    if (ATableCard* Card = Cast<ATableCard>(Target))
        Add(TEXT("flip_card"), Card->bFaceUp ? TEXT("Turn Face Down") : TEXT("Turn Face Up"));

    if (ATableDeck* Deck = Cast<ATableDeck>(Target))
    {
        Add(TEXT("shuffle_deck"), TEXT("Shuffle"));
        Add(TEXT("draw_card"),    TEXT("Draw"));
    }

    return Options;
}

void URadialMenuWidget::ExecuteAction(const FString& ActionID)
{
    if (!CurrentTarget) return;

    if (ActionID == TEXT("flip"))            CurrentTarget->Server_Flip();
    else if (ActionID == TEXT("lock"))       CurrentTarget->Server_Lock(!CurrentTarget->bPhysicsLocked);
    else if (ActionID == TEXT("roll"))
    {
        if (ATableDice* Dice = Cast<ATableDice>(CurrentTarget))
        {
            FVector UpImpulse(FMath::RandRange(-50.f,50.f), FMath::RandRange(-50.f,50.f), 400.f);
            Dice->Server_Roll(UpImpulse);
        }
    }
    else if (ActionID == TEXT("flip_card"))
    {
        if (ATableCard* Card = Cast<ATableCard>(CurrentTarget)) Card->Server_FlipCard();
    }
    else if (ActionID == TEXT("shuffle_deck"))
    {
        if (ATableDeck* Deck = Cast<ATableDeck>(CurrentTarget)) Deck->Server_Shuffle();
    }
    else if (ActionID == TEXT("draw_card"))
    {
        if (ATableDeck* Deck = Cast<ATableDeck>(CurrentTarget)) Deck->Server_DrawTopCard(nullptr);
    }
    else if (ActionID == TEXT("delete"))
    {
        CurrentTarget->Destroy();
    }
    else if (ActionID == TEXT("duplicate"))
    {
        CurrentTarget->Server_Duplicate();
    }
    else if (ActionID == TEXT("ungroup"))
    {
        CurrentTarget->Server_SetGroup(FString());
    }
    // "rotate" remains a continuous drag/gizmo interaction handled by the
    // player controller's grab component, not a discrete radial-menu action.

    Close();
}
