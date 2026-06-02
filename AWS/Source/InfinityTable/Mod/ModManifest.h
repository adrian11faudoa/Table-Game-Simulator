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
    UPROPERTY(BlueprintReadWrite) float   Mass = 0.05f;
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
    UPROPERTY(BlueprintReadWrite) FString EntryScript;
    UPROPERTY(BlueprintReadWrite) TArray<FString> Tags;
    UPROPERTY(BlueprintReadWrite) TArray<FString> Permissions;
    UPROPERTY(BlueprintReadWrite) TArray<FModObjectDef> Objects;
};
