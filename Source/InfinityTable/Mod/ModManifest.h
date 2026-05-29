#pragma once
#include "CoreMinimal.h"
#include "ModManifest.generated.h"

USTRUCT(BlueprintType)
struct FModObjectDef
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) FString ID;
    UPROPERTY(BlueprintReadWrite) FString MeshPath;
    UPROPERTY(BlueprintReadWrite) FString TexturePath;
    UPROPERTY(BlueprintReadWrite) float   Mass        = 0.05f;
    UPROPERTY(BlueprintReadWrite) float   Friction    = 0.6f;
    UPROPERTY(BlueprintReadWrite) float   Restitution = 0.4f;
};

USTRUCT(BlueprintType)
struct FITModManifest
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) FString ID;
    UPROPERTY(BlueprintReadWrite) FString Name;
    UPROPERTY(BlueprintReadWrite) FString Version;
    UPROPERTY(BlueprintReadWrite) FString Author;
    UPROPERTY(BlueprintReadWrite) FString Description;
    UPROPERTY(BlueprintReadWrite) FString EntryScript;   // relative path
    UPROPERTY(BlueprintReadWrite) FString MinEngineVersion = TEXT("0.1.0");
    UPROPERTY(BlueprintReadWrite) TArray<FString>       Tags;
    UPROPERTY(BlueprintReadWrite) TArray<FString>       Permissions;
    UPROPERTY(BlueprintReadWrite) TArray<FModObjectDef> Objects;
};
