#include "ModValidator.h"
#include "Json.h"

TArray<FString> UModValidator::BlockedLuaPatterns;
TArray<FString> UModValidator::BlockedFileExtensions;

void UModValidator::InitBlockLists()
{
    if (BlockedLuaPatterns.Num() > 0) return;

    BlockedLuaPatterns = {
        TEXT("io."),      TEXT("os."),       TEXT("socket"),
        TEXT("ffi"),      TEXT("loadstring"),TEXT("debug."),
        TEXT("package."), TEXT("rawget"),    TEXT("rawset"),
        TEXT("dofile"),   TEXT("loadfile"),  TEXT("load("),
        TEXT("require"),  TEXT("collectgarbage")
    };

    BlockedFileExtensions = {
        TEXT("exe"), TEXT("dll"), TEXT("bat"),
        TEXT("sh"),  TEXT("ps1"), TEXT("cmd"),
        TEXT("com"), TEXT("scr"), TEXT("vbs"),
        TEXT("lnk"), TEXT("msi"), TEXT("app")
    };
}

bool UModValidator::ValidateMod(const FString& ModPath)
{
    InitBlockLists();

    if (!FPaths::DirectoryExists(ModPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModValidator: Path does not exist: %s"), *ModPath);
        return false;
    }

    if (!ValidateManifest(ModPath)) return false;
    if (!ScanLuaFiles(ModPath))     return false;
    if (!ScanAssetFiles(ModPath))   return false;

    return true;
}

bool UModValidator::ValidateManifest(const FString& ModPath)
{
    FString ManifestPath = ModPath / TEXT("manifest.json");
    FString JsonStr;

    if (!FFileHelper::LoadFileToString(JsonStr, *ManifestPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModValidator: No manifest.json in %s"), *ModPath);
        return false;
    }

    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
    if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ModValidator: Invalid JSON in manifest"));
        return false;
    }

    // Required fields
    for (const FString& Field : { FString("id"), FString("name"),
                                   FString("version"), FString("author"), FString("entry") })
    {
        if (!JsonObj->HasField(Field))
        {
            UE_LOG(LogTemp, Warning, TEXT("ModValidator: Missing required field '%s'"), *Field);
            return false;
        }
    }

    // Validate ID: only lowercase alphanumeric + underscore
    FString ID = JsonObj->GetStringField(TEXT("id"));
    for (TCHAR Ch : ID)
    {
        if (!FChar::IsAlpha(Ch) && !FChar::IsDigit(Ch) && Ch != TEXT('_'))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("ModValidator: Invalid ID format '%s' (use [a-z0-9_])"), *ID);
            return false;
        }
    }

    // Validate version format x.y.z
    FString Version = JsonObj->GetStringField(TEXT("version"));
    TArray<FString> Parts;
    Version.ParseIntoArray(Parts, TEXT("."));
    if (Parts.Num() != 3)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ModValidator: Invalid version format '%s' (use x.y.z)"), *Version);
        return false;
    }

    return true;
}

bool UModValidator::ScanLuaFiles(const FString& ModPath)
{
    TArray<FString> LuaFiles;
    IFileManager::Get().FindFilesRecursive(
        LuaFiles, *(ModPath / TEXT("scripts")),
        TEXT("*.lua"), true, false);

    // Also scan root
    TArray<FString> RootLua;
    IFileManager::Get().FindFiles(RootLua, *(ModPath / TEXT("*.lua")), true, false);
    for (const FString& F : RootLua)
        LuaFiles.Add(ModPath / F);

    for (const FString& LuaFile : LuaFiles)
    {
        FString Source;
        if (!FFileHelper::LoadFileToString(Source, *LuaFile)) continue;

        for (const FString& Pattern : BlockedLuaPatterns)
        {
            if (Source.Contains(Pattern))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("ModValidator: Blocked pattern '%s' in %s"), *Pattern, *LuaFile);
                return false;
            }
        }

        // Check file size (max 512 KB per script)
        int64 FileSize = IFileManager::Get().FileSize(*LuaFile);
        if (FileSize > 512 * 1024)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("ModValidator: Script too large (%lld bytes): %s"), FileSize, *LuaFile);
            return false;
        }
    }

    return true;
}

bool UModValidator::ScanAssetFiles(const FString& ModPath)
{
    TArray<FString> AllFiles;
    IFileManager::Get().FindFilesRecursive(
        AllFiles, *ModPath, TEXT("*.*"), true, false);

    for (const FString& File : AllFiles)
    {
        FString Ext = FPaths::GetExtension(File).ToLower();
        if (BlockedFileExtensions.Contains(Ext))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("ModValidator: Blocked file type '.%s': %s"), *Ext, *File);
            return false;
        }
    }

    return true;
}
