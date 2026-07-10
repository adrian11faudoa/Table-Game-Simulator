#include "Assets/AssetImportSubsystem.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"

void UAssetImportSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    IFileManager& FM = IFileManager::Get();
    FString Root = GetImportRootPath();
    if (!FM.DirectoryExists(*Root))
    {
        FM.MakeDirectory(*Root, true);
        FM.MakeDirectory(*(Root / TEXT("Meshes")), true);
        FM.MakeDirectory(*(Root / TEXT("Textures")), true);
        FM.MakeDirectory(*(Root / TEXT("Audio")), true);
        FM.MakeDirectory(*(Root / TEXT("Thumbnails")), true);
    }
}

FString UAssetImportSubsystem::GetImportRootPath() const
{
    return FPaths::ProjectSavedDir() / TEXT("ImportedAssets");
}

bool UAssetImportSubsystem::IsFormatSupported(const FString& Extension) const
{
    FString Ext = Extension.ToLower().Replace(TEXT("."), TEXT(""));
    return SupportedMeshExtensions.Contains(Ext)
        || SupportedTextureExtensions.Contains(Ext)
        || SupportedAudioExtensions.Contains(Ext);
}

EImportedAssetType UAssetImportSubsystem::GetTypeForExtension(const FString& Extension) const
{
    FString Ext = Extension.ToLower().Replace(TEXT("."), TEXT(""));
    if (SupportedMeshExtensions.Contains(Ext))    return EImportedAssetType::Mesh3D;
    if (SupportedTextureExtensions.Contains(Ext)) return EImportedAssetType::Texture;
    return EImportedAssetType::Audio;
}

EAssetImportResult UAssetImportSubsystem::ImportAsset(const FString& FilePath, FImportedAssetInfo& OutInfo)
{
    IFileManager& FM = IFileManager::Get();

    if (!FM.FileExists(*FilePath))
    {
        OnAssetImported.Broadcast(FImportedAssetInfo(), EAssetImportResult::FileNotFound);
        return EAssetImportResult::FileNotFound;
    }

    FString Ext = FPaths::GetExtension(FilePath);
    if (!IsFormatSupported(Ext))
    {
        OnAssetImported.Broadcast(FImportedAssetInfo(), EAssetImportResult::UnsupportedFormat);
        return EAssetImportResult::UnsupportedFormat;
    }

    int64 FileSize = FM.FileSize(*FilePath);
    EImportedAssetType Type = GetTypeForExtension(Ext);

    int64 MaxSize = (Type == EImportedAssetType::Mesh3D)  ? MaxMeshFileSizeBytes
                   : (Type == EImportedAssetType::Texture) ? MaxTextureFileSizeBytes
                   : MaxAudioFileSizeBytes;

    if (FileSize <= 0 || FileSize > MaxSize)
    {
        OnAssetImported.Broadcast(FImportedAssetInfo(), EAssetImportResult::FileTooLarge);
        return EAssetImportResult::FileTooLarge;
    }

    OutInfo.AssetID         = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    OutInfo.DisplayName     = FPaths::GetBaseFilename(FilePath);
    OutInfo.SourceFilePath  = FilePath;
    OutInfo.AssetType       = Type;
    OutInfo.FileSizeBytes   = FileSize;
    OutInfo.Category        = (Type == EImportedAssetType::Mesh3D)  ? TEXT("Custom Meshes")
                             : (Type == EImportedAssetType::Texture) ? TEXT("Custom Textures")
                             : TEXT("Custom Audio");

    bool bImported = false;
    switch (Type)
    {
        case EImportedAssetType::Mesh3D:  bImported = ImportMesh(FilePath, OutInfo);    break;
        case EImportedAssetType::Texture: bImported = ImportTexture(FilePath, OutInfo); break;
        case EImportedAssetType::Audio:   bImported = ImportAudio(FilePath, OutInfo);   break;
    }

    if (!bImported)
    {
        OnAssetImported.Broadcast(OutInfo, EAssetImportResult::ImportFailed);
        return EAssetImportResult::ImportFailed;
    }

    OutInfo.ThumbnailPath = GenerateThumbnail(FilePath, Type);
    ImportedAssets.Add(OutInfo.AssetID, OutInfo);

    OnAssetImported.Broadcast(OutInfo, EAssetImportResult::Success);
    return EAssetImportResult::Success;
}

bool UAssetImportSubsystem::ImportMesh(const FString& FilePath, FImportedAssetInfo& OutInfo)
{
    FString Ext = FPaths::GetExtension(FilePath).ToLower();
    FString DestDir = GetImportRootPath() / TEXT("Meshes") / OutInfo.AssetID;
    IFileManager::Get().MakeDirectory(*DestDir, true);
    FString DestPath = DestDir / FPaths::GetCleanFilename(FilePath);

    bool bCopyOK = (IFileManager::Get().Copy(*DestPath, *FilePath) == COPY_OK);
    if (!bCopyOK) return false;

    // GLTF/GLB are runtime-loadable via the glTFRuntime plugin (already an
    // engine dependency — see InfinityTable.uproject). Actual mesh
    // construction from the staged file happens in gameplay code that spawns
    // the ATableObject, since it needs a live UWorld + component to attach
    // the generated mesh to; this subsystem's job ends at "the file is
    // validated, staged, and catalogued for the object browser."
    //
    // FBX and OBJ have no first-party *runtime* (non-editor) import path in
    // stock UE5 without either Interchange's editor-only pipeline or a
    // commercial runtime-FBX SDK. We still accept and catalogue the file
    // (useful for editor-time import ahead of packaging, or a future
    // server-side offline conversion step to GLTF), but it will not appear
    // as a spawnable mesh in a live session until converted. This limitation
    // is documented in Docs/Asset_Import_Guide.md.
    OutInfo.SourceFilePath = DestPath;
    return true;
}

bool UAssetImportSubsystem::ImportTexture(const FString& FilePath, FImportedAssetInfo& OutInfo)
{
    FString DestDir = GetImportRootPath() / TEXT("Textures") / OutInfo.AssetID;
    IFileManager::Get().MakeDirectory(*DestDir, true);
    FString DestPath = DestDir / FPaths::GetCleanFilename(FilePath);

    bool bCopyOK = (IFileManager::Get().Copy(*DestPath, *FilePath) == COPY_OK);
    if (!bCopyOK) return false;

    OutInfo.SourceFilePath = DestPath;
    return true;
}

bool UAssetImportSubsystem::ImportAudio(const FString& FilePath, FImportedAssetInfo& OutInfo)
{
    FString DestDir = GetImportRootPath() / TEXT("Audio") / OutInfo.AssetID;
    IFileManager::Get().MakeDirectory(*DestDir, true);
    FString DestPath = DestDir / FPaths::GetCleanFilename(FilePath);

    bool bCopyOK = (IFileManager::Get().Copy(*DestPath, *FilePath) == COPY_OK);
    if (!bCopyOK) return false;

    OutInfo.SourceFilePath = DestPath;
    return true;
}

FString UAssetImportSubsystem::GenerateThumbnail(const FString& FilePath, EImportedAssetType Type)
{
    // Textures can thumbnail themselves directly. Meshes and audio need a
    // rendered/generated thumbnail (e.g. a SceneCapture2D render of the
    // imported mesh, or a generic waveform/note icon for audio) — that
    // requires a live render target and is wired up on the UI side
    // (UObjectBrowserWidget) rather than here, since this subsystem has no
    // guaranteed access to a renderable UWorld at import time (e.g. if
    // called from a background import queue). Returns an empty path when
    // no thumbnail can be generated synchronously.
    if (Type == EImportedAssetType::Texture)
    {
        return FilePath;
    }
    return FString();
}

TArray<FImportedAssetInfo> UAssetImportSubsystem::GetImportedAssets(EImportedAssetType FilterType) const
{
    TArray<FImportedAssetInfo> Out;
    for (const auto& Pair : ImportedAssets)
    {
        if (Pair.Value.AssetType == FilterType)
            Out.Add(Pair.Value);
    }
    return Out;
}

TArray<FImportedAssetInfo> UAssetImportSubsystem::GetAllImportedAssets() const
{
    TArray<FImportedAssetInfo> Out;
    ImportedAssets.GenerateValueArray(Out);
    return Out;
}

bool UAssetImportSubsystem::RemoveImportedAsset(const FString& AssetID)
{
    if (const FImportedAssetInfo* Info = ImportedAssets.Find(AssetID))
    {
        IFileManager::Get().DeleteDirectory(*FPaths::GetPath(Info->SourceFilePath), false, true);
        return ImportedAssets.Remove(AssetID) > 0;
    }
    return false;
}
