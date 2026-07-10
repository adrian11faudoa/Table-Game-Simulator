#include "Mod/ModLoadingSubsystem.h"
#include "Scripting/LuaSubsystem.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "JsonObjectConverter.h"

void UModLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    IFileManager& FM = IFileManager::Get();
    FString Root = GetModsRootPath();
    if (!FM.DirectoryExists(*Root))
    {
        FM.MakeDirectory(*Root, true);
    }
}

FString UModLoadingSubsystem::GetModsRootPath() const
{
    return FPaths::ProjectSavedDir() / ModsDirectoryName;
}

TArray<FString> UModLoadingSubsystem::DiscoverMods() const
{
    TArray<FString> Found;
    IFileManager& FM = IFileManager::Get();
    FString Root = GetModsRootPath();

    TArray<FString> Dirs;
    FM.IterateDirectory(*Root, [&Dirs](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
    {
        if (bIsDirectory) Dirs.Add(FString(FilenameOrDirectory));
        return true;
    });

    for (const FString& Dir : Dirs)
    {
        FString ManifestPath = Dir / TEXT("manifest.json");
        if (FM.FileExists(*ManifestPath))
        {
            Found.Add(Dir);
        }
    }
    return Found;
}

bool UModLoadingSubsystem::ParseManifest(const FString& JsonPath, FITModManifest& OutManifest) const
{
    FString JsonText;
    if (!FFileHelper::LoadFileToString(JsonText, *JsonPath)) return false;
    return FJsonObjectConverter::JsonObjectStringToUStruct(JsonText, &OutManifest, 0, 0);
}

bool UModLoadingSubsystem::ScanLuaForBlockedPatterns(const FString& ScriptPath, FString& OutError) const
{
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *ScriptPath))
    {
        OutError = TEXT("Could not read script file");
        return false;
    }
    for (const FString& Pattern : BlockedScriptPatterns)
    {
        if (Source.Contains(Pattern))
        {
            OutError = FString::Printf(TEXT("Script contains blocked pattern: %s"), *Pattern);
            return false;
        }
    }
    return true;
}

bool UModLoadingSubsystem::ValidateMod(const FString& ModFolder, FString& OutError) const
{
    IFileManager& FM = IFileManager::Get();
    FString ManifestPath = ModFolder / TEXT("manifest.json");
    if (!FM.FileExists(*ManifestPath))
    {
        OutError = TEXT("manifest.json not found");
        return false;
    }

    FITModManifest Manifest;
    if (!ParseManifest(ManifestPath, Manifest))
    {
        OutError = TEXT("manifest.json is malformed");
        return false;
    }

    if (Manifest.ID.IsEmpty() || Manifest.EntryScript.IsEmpty())
    {
        OutError = TEXT("manifest.json missing required fields (id, entry)");
        return false;
    }

    // Block executable file types anywhere in the mod folder
    TArray<FString> AllFiles;
    FM.FindFilesRecursive(AllFiles, *ModFolder, TEXT("*.*"), true, false);
    for (const FString& File : AllFiles)
    {
        FString Ext = FPaths::GetExtension(File, true).ToLower();
        if (BlockedFileExtensions.Contains(Ext))
        {
            OutError = FString::Printf(TEXT("Mod contains blocked file type: %s"), *File);
            return false;
        }
    }

    // Scan the entry script (and any other .lua files) for blocked patterns
    FString EntryPath = ModFolder / Manifest.EntryScript;
    if (!FM.FileExists(*EntryPath))
    {
        OutError = TEXT("Entry script not found");
        return false;
    }

    for (const FString& File : AllFiles)
    {
        if (FPaths::GetExtension(File).ToLower() == TEXT("lua"))
        {
            FString ScanError;
            if (!ScanLuaForBlockedPatterns(File, ScanError))
            {
                OutError = ScanError;
                return false;
            }
        }
    }

    return true;
}

bool UModLoadingSubsystem::LoadMod(const FString& ModFolder)
{
    FString Error;
    if (!ValidateMod(ModFolder, Error))
    {
        UE_LOG(LogTemp, Error, TEXT("ModLoadingSubsystem: Failed to load mod at %s: %s"), *ModFolder, *Error);
        return false;
    }

    FITModManifest Manifest;
    if (!ParseManifest(ModFolder / TEXT("manifest.json"), Manifest))
        return false;

    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
    {
        FString EntryPath = ModFolder / Manifest.EntryScript;
        if (!Lua->LoadScript(EntryPath))
        {
            UE_LOG(LogTemp, Error, TEXT("ModLoadingSubsystem: Lua failed to load entry script for mod %s"), *Manifest.ID);
            return false;
        }
    }

    LoadedMods.Add(Manifest.ID, Manifest);
    UE_LOG(LogTemp, Log, TEXT("ModLoadingSubsystem: Loaded mod '%s' v%s"), *Manifest.Name, *Manifest.Version);
    return true;
}

bool UModLoadingSubsystem::UnloadMod(const FString& ModID)
{
    return LoadedMods.Remove(ModID) > 0;
}

TArray<FITModManifest> UModLoadingSubsystem::GetLoadedMods() const
{
    TArray<FITModManifest> Out;
    LoadedMods.GenerateValueArray(Out);
    return Out;
}

bool UModLoadingSubsystem::HasPermission(const FString& ModID, const FString& Permission) const
{
    if (const FITModManifest* Manifest = LoadedMods.Find(ModID))
    {
        return Manifest->Permissions.Contains(Permission);
    }
    return false;
}
