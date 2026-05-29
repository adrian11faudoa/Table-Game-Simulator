#include "ModLoadingSubsystem.h"
#include "ModValidator.h"
#include "Scripting/LuaSubsystem.h"
#include "Objects/TableSpawnManager.h"
#include "Json.h"
#include "JsonObjectConverter.h"

void UModLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("ModLoadingSubsystem: Ready. Mods dir: %s"), *GetModsDirectory());
}

FString UModLoadingSubsystem::GetModsDirectory() const
{
    return FPaths::ProjectDir() / TEXT("Mods");
}

void UModLoadingSubsystem::ScanAndLoadAll()
{
    TArray<FString> ModDirs;
    IFileManager::Get().FindFiles(ModDirs, *(GetModsDirectory() / TEXT("*")), false, true);

    for (const FString& Dir : ModDirs)
    {
        FString FullPath = GetModsDirectory() / Dir;
        LoadMod(FullPath);
    }
}

bool UModLoadingSubsystem::LoadMod(const FString& ModPath)
{
    // 1. Validate
    if (!UModValidator::ValidateMod(ModPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModLoader: Validation FAILED for %s"), *ModPath);
        OnModLoaded.Broadcast(FPaths::GetCleanFilename(ModPath), false);
        return false;
    }

    // 2. Parse manifest
    FITModManifest Manifest;
    if (!ParseManifest(ModPath, Manifest))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModLoader: Cannot parse manifest in %s"), *ModPath);
        OnModLoaded.Broadcast(FPaths::GetCleanFilename(ModPath), false);
        return false;
    }

    if (IsModLoaded(Manifest.ID))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModLoader: Mod %s already loaded"), *Manifest.ID);
        return false;
    }

    // 3. Build loaded mod record
    FLoadedMod Mod;
    Mod.Manifest = Manifest;
    Mod.ModPath  = ModPath;
    Mod.State    = EModState::Loading;
    Mod.LoadedAt = FDateTime::UtcNow();

    // 4. Register objects
    if (!LoadModAssets(Mod))
    {
        UE_LOG(LogTemp, Warning, TEXT("ModLoader: Asset load FAILED for %s"), *Manifest.ID);
        OnModLoaded.Broadcast(Manifest.ID, false);
        return false;
    }

    // 5. Execute Lua entry script
    if (!Manifest.EntryScript.IsEmpty())
    {
        if (!ExecuteModScript(Mod))
        {
            UE_LOG(LogTemp, Warning, TEXT("ModLoader: Script error in %s"), *Manifest.ID);
            OnModLoaded.Broadcast(Manifest.ID, false);
            return false;
        }
    }

    Mod.State = EModState::Loaded;
    LoadedMods.Add(Manifest.ID, Mod);

    UE_LOG(LogTemp, Log, TEXT("ModLoader: Loaded mod '%s' v%s by %s"),
        *Manifest.Name, *Manifest.Version, *Manifest.Author);

    OnModLoaded.Broadcast(Manifest.ID, true);
    return true;
}

bool UModLoadingSubsystem::UnloadMod(const FString& ModID)
{
    if (!LoadedMods.Contains(ModID)) return false;
    LoadedMods.Remove(ModID);
    OnModUnloaded.Broadcast(ModID);
    return true;
}

bool UModLoadingSubsystem::IsModLoaded(const FString& ModID) const
{
    return LoadedMods.Contains(ModID);
}

TArray<FLoadedMod> UModLoadingSubsystem::GetLoadedMods() const
{
    TArray<FLoadedMod> Result;
    LoadedMods.GenerateValueArray(Result);
    return Result;
}

bool UModLoadingSubsystem::ParseManifest(const FString& ModPath, FITModManifest& OutManifest)
{
    FString ManifestPath = ModPath / TEXT("manifest.json");
    FString JsonStr;
    if (!FFileHelper::LoadFileToString(JsonStr, *ManifestPath)) return false;

    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
    if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid()) return false;

    OutManifest.ID          = JsonObj->GetStringField(TEXT("id"));
    OutManifest.Name        = JsonObj->GetStringField(TEXT("name"));
    OutManifest.Version     = JsonObj->GetStringField(TEXT("version"));
    OutManifest.Author      = JsonObj->GetStringField(TEXT("author"));
    OutManifest.Description = JsonObj->GetStringField(TEXT("description"));
    OutManifest.EntryScript = JsonObj->GetStringField(TEXT("entry"));

    if (JsonObj->HasField(TEXT("tags")))
    {
        for (const auto& Tag : JsonObj->GetArrayField(TEXT("tags")))
            OutManifest.Tags.Add(Tag->AsString());
    }

    if (JsonObj->HasField(TEXT("permissions")))
    {
        for (const auto& Perm : JsonObj->GetArrayField(TEXT("permissions")))
            OutManifest.Permissions.Add(Perm->AsString());
    }

    return !OutManifest.ID.IsEmpty() && !OutManifest.EntryScript.IsEmpty();
}

bool UModLoadingSubsystem::LoadModAssets(const FLoadedMod& Mod)
{
    UTableSpawnManager* SM = GetGameInstance()->GetSubsystem<UTableSpawnManager>();
    if (!SM) return true; // Not fatal

    for (const FModObjectDef& ObjDef : Mod.Manifest.Objects)
    {
        FObjectDefinition Def;
        Def.TypeID      = Mod.Manifest.ID + TEXT(".") + ObjDef.ID;
        Def.Class       = ATableObject::StaticClass();
        Def.Mass        = ObjDef.Mass;
        Def.Friction    = ObjDef.Friction;
        Def.Restitution = ObjDef.Restitution;

        if (!ObjDef.MeshPath.IsEmpty())
        {
            FString FullMeshPath = Mod.ModPath / ObjDef.MeshPath;
            // TODO: Load via RuntimeMeshImporter
        }

        SM->RegisterObjectType(Def);
    }
    return true;
}

bool UModLoadingSubsystem::ExecuteModScript(const FLoadedMod& Mod)
{
    ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>();
    if (!Lua || !Lua->IsInitialized()) return false;

    FString ScriptPath = Mod.ModPath / Mod.Manifest.EntryScript;
    return Lua->LoadScript(ScriptPath);
}
