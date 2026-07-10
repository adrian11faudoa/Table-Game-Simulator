#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AssetImportSubsystem.generated.h"

UENUM(BlueprintType)
enum class EImportedAssetType : uint8
{
    Mesh3D,      // FBX, OBJ, GLTF/GLB
    Texture,     // PNG, JPG, WEBP
    Audio        // MP3, WAV
};

UENUM(BlueprintType)
enum class EAssetImportResult : uint8
{
    Success,
    UnsupportedFormat,
    FileTooLarge,
    FileNotFound,
    ImportFailed
};

USTRUCT(BlueprintType)
struct FImportedAssetInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString AssetID;
    UPROPERTY(BlueprintReadOnly) FString DisplayName;
    UPROPERTY(BlueprintReadOnly) FString SourceFilePath;
    UPROPERTY(BlueprintReadOnly) EImportedAssetType AssetType = EImportedAssetType::Mesh3D;
    UPROPERTY(BlueprintReadOnly) FString Category = TEXT("Imported");
    UPROPERTY(BlueprintReadOnly) FString ThumbnailPath;
    UPROPERTY(BlueprintReadOnly) int64 FileSizeBytes = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssetImported, FImportedAssetInfo, AssetInfo, EAssetImportResult, Result);

/**
 * Handles drag-and-drop / file-picker import of user assets at runtime
 * (as opposed to editor-time FBX/OBJ import, which UE5 already provides
 * out of the box). 3D meshes are routed through the glTFRuntime plugin
 * for GLTF/GLB, since that's the only one of the three required mesh
 * formats (FBX/OBJ/GLTF) with a viable *runtime* (non-editor) importer
 * available without a commercial SDK — see Docs/Asset_Import_Guide.md
 * for the full breakdown of what is and isn't supported at runtime
 * versus import-time-only via the editor.
 */
UCLASS()
class INFINITYTABLE_API UAssetImportSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable) EAssetImportResult ImportAsset(const FString& FilePath, FImportedAssetInfo& OutInfo);
    UFUNCTION(BlueprintCallable) TArray<FImportedAssetInfo> GetImportedAssets(EImportedAssetType FilterType) const;
    UFUNCTION(BlueprintCallable) TArray<FImportedAssetInfo> GetAllImportedAssets() const;
    UFUNCTION(BlueprintCallable) bool RemoveImportedAsset(const FString& AssetID);
    UFUNCTION(BlueprintCallable) bool IsFormatSupported(const FString& Extension) const;
    UFUNCTION(BlueprintCallable) EImportedAssetType GetTypeForExtension(const FString& Extension) const;

    UPROPERTY(BlueprintAssignable) FOnAssetImported OnAssetImported;

    UPROPERTY(EditDefaultsOnly) int64 MaxMeshFileSizeBytes    = 64  * 1024 * 1024; // 64 MB
    UPROPERTY(EditDefaultsOnly) int64 MaxTextureFileSizeBytes = 16  * 1024 * 1024; // 16 MB
    UPROPERTY(EditDefaultsOnly) int64 MaxAudioFileSizeBytes   = 32  * 1024 * 1024; // 32 MB

    UPROPERTY(EditDefaultsOnly) TArray<FString> SupportedMeshExtensions    = { TEXT("gltf"), TEXT("glb"), TEXT("fbx"), TEXT("obj") };
    UPROPERTY(EditDefaultsOnly) TArray<FString> SupportedTextureExtensions = { TEXT("png"), TEXT("jpg"), TEXT("jpeg"), TEXT("webp") };
    UPROPERTY(EditDefaultsOnly) TArray<FString> SupportedAudioExtensions   = { TEXT("mp3"), TEXT("wav") };

private:
    UPROPERTY() TMap<FString, FImportedAssetInfo> ImportedAssets;
    FString GetImportRootPath() const;
    FString GenerateThumbnail(const FString& FilePath, EImportedAssetType Type);
    bool ImportMesh(const FString& FilePath, FImportedAssetInfo& OutInfo);
    bool ImportTexture(const FString& FilePath, FImportedAssetInfo& OutInfo);
    bool ImportAudio(const FString& FilePath, FImportedAssetInfo& OutInfo);
};
