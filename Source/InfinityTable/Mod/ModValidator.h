#pragma once
#include "CoreMinimal.h"

class INFINITYTABLE_API UModValidator
{
public:
    static bool ValidateMod(const FString& ModPath);

private:
    static bool ValidateManifest(const FString& ModPath);
    static bool ScanLuaFiles(const FString& ModPath);
    static bool ScanAssetFiles(const FString& ModPath);

    static TArray<FString> BlockedLuaPatterns;
    static TArray<FString> BlockedFileExtensions;
    static void InitBlockLists();
};
