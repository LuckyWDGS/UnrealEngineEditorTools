// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "EditorToolsBPFLibrary.h"
#include "Logging/EditorToolsLog.h"
#include "Logging/EditorToolsMessageLog.h"
#include "Logging/ActorSelectToken.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Misc/UObjectToken.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/Material.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "MaterialShared.h"
#include "Materials/MaterialInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/MapBuildDataRegistry.h"
#include "StaticMeshComponentLODInfo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/SkyLight.h"
#include "Engine/RectLight.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/RectLightComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/SkeletalMesh.h"
#include "ObjectTools.h"                                                                                              
#include "AssetDeleteModel.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "Engine/Selection.h"
#include "Engine/CollisionProfile.h"
#include "EditorToolsUtilities.h"
#include "Types/CollisionModeTextTypes.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Engine/Texture2D.h"
#include "Logging/AssetObjectToken.h"


#define LOCTEXT_NAMESPACE "FEditorToolsBPFLibrary"

namespace
{
	static UStaticMesh* ResolveMeshFromAnimation(const UAnimationAsset* Animation)
	{
		return nullptr;
	}
}

TArray<UMaterialInterface*> UEditorToolsBPFLibrary::GetStaticMeshMaterials(UStaticMesh* StaticMesh)
{
	TArray<UMaterialInterface*> Materials;
	
	if (StaticMesh)
	{
		const TArray<FStaticMaterial>& StaticMaterials = StaticMesh->GetStaticMaterials();
		Materials.Reserve(StaticMaterials.Num());
		
		for (const FStaticMaterial& StaticMaterial : StaticMaterials)
		{
			if (StaticMaterial.MaterialInterface)
			{
				Materials.Add(StaticMaterial.MaterialInterface);
			}
		}
	}
	
	return Materials;
}

UMaterialInstanceConstant* UEditorToolsBPFLibrary::CreateMaterialInstanceFromBase(UMaterialInterface* BaseMaterial, const FString& SavePath)
{
	// 检查基础材质是否有效
	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMaterialInstanceFromBase: BaseMaterial is null"));
		return nullptr;
	}

	// 检查路径是否有效
	if (SavePath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMaterialInstanceFromBase: SavePath is empty"));
		return nullptr;
	}

	// 分离路径和资源名称
	FString PackagePath;
	FString AssetName;
	
	// 如果路径包含资源名称，则分离；否则使用默认名称
	if (SavePath.Contains(TEXT("/")))
	{
		int32 LastSlashIndex;
		SavePath.FindLastChar(TEXT('/'), LastSlashIndex);
		PackagePath = SavePath.Left(LastSlashIndex);
		AssetName = SavePath.Mid(LastSlashIndex + 1);
	}
	else
	{
		PackagePath = SavePath;
		AssetName = TEXT("MI_NewMaterialInstance");
	}

	// 确保路径以 /Game/ 开头
	if (!PackagePath.StartsWith(TEXT("/Game/")))
	{
		if (PackagePath.StartsWith(TEXT("/")))
		{
			PackagePath = TEXT("/Game") + PackagePath;
		}
		else
		{
			PackagePath = TEXT("/Game/") + PackagePath;
		}
	}

	// 获取 AssetTools 模块
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// 创建材质实例工厂
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = BaseMaterial;

	// 创建资源
	UObject* NewAsset = AssetTools.CreateAsset(AssetName, PackagePath, UMaterialInstanceConstant::StaticClass(), Factory);
	
	UMaterialInstanceConstant* NewMaterialInstance = Cast<UMaterialInstanceConstant>(NewAsset);
	
	if (NewMaterialInstance)
	{
		// 标记为已修改
		NewMaterialInstance->MarkPackageDirty();
		
		// 保存资源包
		FString PackageName = PackagePath + TEXT("/") + AssetName;
		UPackage* Package = NewMaterialInstance->GetOutermost();
		if (Package)
		{
			Package->SetDirtyFlag(true);
			FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
			
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
			SaveArgs.Error = GError;
			SaveArgs.bForceByteSwapping = true;
			SaveArgs.bWarnOfLongFilename = true;
			SaveArgs.SaveFlags = SAVE_NoError;
			
			UPackage::SavePackage(Package, NewMaterialInstance, *PackageFilename, SaveArgs);
		}
		
		UE_LOG(LogTemp, Log, TEXT("CreateMaterialInstanceFromBase: Successfully created material instance at %s"), *PackageName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMaterialInstanceFromBase: Failed to create material instance"));
	}

	return NewMaterialInstance;
}

void UEditorToolsBPFLibrary::SetMICTexture(UMaterialInterface* MaterialInterface, FName ParamName, UTexture* NewTexture)
{
	if (!MaterialInterface || !NewTexture) return;

	if (UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(MaterialInterface))
	{
		FMaterialParameterInfo Info(ParamName);
		MIC->SetTextureParameterValueEditorOnly(Info, NewTexture);
		MIC->PostEditChange();
		MIC->MarkPackageDirty();
		// 如需立刻落盘可 SavePackage(MaterialInterface->GetOutermost(), MaterialInterface, ...)
		FString PkgFilename = FPackageName::LongPackageNameToFilename(MaterialInterface->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
		SaveArgs.SaveFlags = SAVE_None;
		UPackage::SavePackage(MaterialInterface->GetOutermost(), MaterialInterface, *PkgFilename, SaveArgs);
		
	}
	
}

void UEditorToolsBPFLibrary::SetStaticMeshComponentMaterial(UStaticMeshComponent* MeshComponent, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetStaticMeshComponentMaterial: MeshComponent is null"));
		return;
	}

	if (!Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetStaticMeshComponentMaterial: Material is null"));
		return;
	}

	// 检查材质索引是否有效
	if (MaterialIndex < 0 || MaterialIndex >= MeshComponent->GetNumMaterials())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetStaticMeshComponentMaterial: Invalid MaterialIndex %d. Component has %d materials."), 
			MaterialIndex, MeshComponent->GetNumMaterials());
		return;
	}

	// 设置组件实例的材质，不影响资产
	MeshComponent->SetMaterial(MaterialIndex, Material);
	
	// 标记组件需要重新渲染
	MeshComponent->MarkRenderStateDirty();
	
	UE_LOG(LogTemp, Log, TEXT("SetStaticMeshComponentMaterial: Successfully set material at index %d for component %s"), 
		MaterialIndex, *MeshComponent->GetName());
}

UMaterialInterface* UEditorToolsBPFLibrary::GetStaticMeshComponentMaterial(UStaticMeshComponent* MeshComponent, int32 MaterialIndex)
{
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetStaticMeshComponentMaterial: MeshComponent is null"));
		return nullptr;
	}

	// 检查材质索引是否有效
	if (MaterialIndex < 0 || MaterialIndex >= MeshComponent->GetNumMaterials())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetStaticMeshComponentMaterial: Invalid MaterialIndex %d. Component has %d materials."), 
			MaterialIndex, MeshComponent->GetNumMaterials());
		return nullptr;
	}

	return MeshComponent->GetMaterial(MaterialIndex);
}

TArray<UMaterialInterface*> UEditorToolsBPFLibrary::GetStaticMeshComponentMaterials(UStaticMeshComponent* MeshComponent)
{
	TArray<UMaterialInterface*> Materials;
	
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetStaticMeshComponentMaterials: MeshComponent is null"));
		return Materials;
	}

	int32 NumMaterials = MeshComponent->GetNumMaterials();
	Materials.Reserve(NumMaterials);

	for (int32 i = 0; i < NumMaterials; i++)
	{
		UMaterialInterface* Mat = MeshComponent->GetMaterial(i);
		Materials.Add(Mat);
	}

	return Materials;
}

void UEditorToolsBPFLibrary::SetActorStaticMeshesMaterial(AActor* Actor, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetActorStaticMeshesMaterial: Actor is null"));
		return;
	}

	if (!Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetActorStaticMeshesMaterial: Material is null"));
		return;
	}

	// 获取Actor上所有的静态网格体组件
	TArray<UStaticMeshComponent*> MeshComponents;
	Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

	if (MeshComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetActorStaticMeshesMaterial: No StaticMeshComponents found on Actor %s"), *Actor->GetName());
		return;
	}

	int32 UpdatedCount = 0;
	for (UStaticMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp && MaterialIndex < MeshComp->GetNumMaterials())
		{
			MeshComp->SetMaterial(MaterialIndex, Material);
			MeshComp->MarkRenderStateDirty();
			UpdatedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SetActorStaticMeshesMaterial: Updated material on %d components for Actor %s"), 
		UpdatedCount, *Actor->GetName());
}

UTexture* UEditorToolsBPFLibrary::LoadTextureFromPath(const FString& AssetPath, const FString& AssetName)
{
	// 检查输入是否有效
	if (AssetPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("LoadTextureFromPath: AssetPath is empty"));
		return nullptr;
	}

	if (AssetName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("LoadTextureFromPath: AssetName is empty"));
		return nullptr;
	}

	// 构建完整的资源路径
	FString FullPath = AssetPath;
	
	// 确保路径以 /Game/ 开头
	if (!FullPath.StartsWith(TEXT("/Game/")))
	{
		if (FullPath.StartsWith(TEXT("/")))
		{
			FullPath = TEXT("/Game") + FullPath;
		}
		else if (FullPath.StartsWith(TEXT("Game/")))
		{
			FullPath = TEXT("/") + FullPath;
		}
		else
		{
			FullPath = TEXT("/Game/") + FullPath;
		}
	}

	// 移除路径末尾的斜杠（如果有）
	if (FullPath.EndsWith(TEXT("/")))
	{
		FullPath = FullPath.Left(FullPath.Len() - 1);
	}

	// 组合完整路径
	FullPath = FullPath + TEXT("/") + AssetName;

	// 移除文件扩展名（如果用户提供了）
	FString CleanAssetName = AssetName;
	if (CleanAssetName.Contains(TEXT(".")))
	{
		int32 DotIndex;
		CleanAssetName.FindLastChar(TEXT('.'), DotIndex);
		CleanAssetName = CleanAssetName.Left(DotIndex);
		
		// 重新构建路径
		FullPath = AssetPath;
		if (!FullPath.StartsWith(TEXT("/Game/")))
		{
			if (FullPath.StartsWith(TEXT("/")))
			{
				FullPath = TEXT("/Game") + FullPath;
			}
			else if (FullPath.StartsWith(TEXT("Game/")))
			{
				FullPath = TEXT("/") + FullPath;
			}
			else
			{
				FullPath = TEXT("/Game/") + FullPath;
			}
		}
		if (FullPath.EndsWith(TEXT("/")))
		{
			FullPath = FullPath.Left(FullPath.Len() - 1);
		}
		FullPath = FullPath + TEXT("/") + CleanAssetName;
	}

	// 添加资源引用格式（ObjectPath.ObjectName）
	FString AssetReference = FullPath + TEXT(".") + (CleanAssetName.IsEmpty() ? AssetName : CleanAssetName);

	// 尝试加载纹理
	UTexture* LoadedTexture = LoadObject<UTexture>(nullptr, *AssetReference);

	if (LoadedTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("LoadTextureFromPath: Successfully loaded texture from %s"), *AssetReference);
		return LoadedTexture;
	}

	// 如果加载失败，尝试不带对象名的路径
	LoadedTexture = LoadObject<UTexture>(nullptr, *FullPath);
	
	if (LoadedTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("LoadTextureFromPath: Successfully loaded texture from %s"), *FullPath);
		return LoadedTexture;
	}

	// 尝试使用 StaticLoadObject
	LoadedTexture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), nullptr, *AssetReference));
	
	if (LoadedTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("LoadTextureFromPath: Successfully loaded texture using StaticLoadObject from %s"), *AssetReference);
		return LoadedTexture;
	}

	UE_LOG(LogTemp, Error, TEXT("LoadTextureFromPath: Failed to load texture from path: %s (tried: %s)"), *FullPath, *AssetReference);
	return nullptr;
}

UTexture* UEditorToolsBPFLibrary::GetTextureFromMaterial(UMaterialInterface* MaterialInterface, FName ParameterName)
{
	if (!MaterialInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetTextureFromMaterial: MaterialInterface is null"));
		return nullptr;
	}

	if (ParameterName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetTextureFromMaterial: ParameterName is None"));
		return nullptr;
	}

	UTexture* OutTexture = nullptr;

	// 尝试从材质实例中获取纹理参数
	if (UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface))
	{
		if (MaterialInstance->GetTextureParameterValue(ParameterName, OutTexture))
		{
			UE_LOG(LogTemp, Log, TEXT("GetTextureFromMaterial: Found texture '%s' in MaterialInstance '%s'"), 
				*ParameterName.ToString(), *MaterialInterface->GetName());
			return OutTexture;
		}
	}

	// 尝试使用通用方法获取纹理
	MaterialInterface->GetTextureParameterValue(ParameterName, OutTexture);
	
	if (OutTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("GetTextureFromMaterial: Found texture '%s' in Material '%s'"), 
			*ParameterName.ToString(), *MaterialInterface->GetName());
		return OutTexture;
	}

	UE_LOG(LogTemp, Warning, TEXT("GetTextureFromMaterial: Texture parameter '%s' not found in Material '%s'"), 
		*ParameterName.ToString(), *MaterialInterface->GetName());
	return nullptr;
}

TArray<FName> UEditorToolsBPFLibrary::GetAllTextureParameterNames(UMaterialInterface* MaterialInterface)
{
	TArray<FName> TextureParameterNames;

	if (!MaterialInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllTextureParameterNames: MaterialInterface is null"));
		return TextureParameterNames;
	}

	TArray<FMaterialParameterInfo> OutParameterInfo;
	TArray<FGuid> OutParameterIds;

	// 获取所有纹理参数信息
	MaterialInterface->GetAllTextureParameterInfo(OutParameterInfo, OutParameterIds);

	// 提取参数名称
	TextureParameterNames.Reserve(OutParameterInfo.Num());
	for (const FMaterialParameterInfo& ParamInfo : OutParameterInfo)
	{
		TextureParameterNames.Add(ParamInfo.Name);
	}

	UE_LOG(LogTemp, Log, TEXT("GetAllTextureParameterNames: Found %d texture parameters in Material '%s'"), 
		TextureParameterNames.Num(), *MaterialInterface->GetName());

	return TextureParameterNames;
}

TMap<FName, UTexture*> UEditorToolsBPFLibrary::GetAllTexturesFromMaterial(UMaterialInterface* MaterialInterface)
{
	TMap<FName, UTexture*> TextureMap;

	if (!MaterialInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllTexturesFromMaterial: MaterialInterface is null"));
		return TextureMap;
	}

	TArray<FMaterialParameterInfo> OutParameterInfo;
	TArray<FGuid> OutParameterIds;

	// 获取所有纹理参数信息
	MaterialInterface->GetAllTextureParameterInfo(OutParameterInfo, OutParameterIds);

	// 遍历每个参数并获取纹理值
	for (const FMaterialParameterInfo& ParamInfo : OutParameterInfo)
	{
		UTexture* Texture = nullptr;
		if (MaterialInterface->GetTextureParameterValue(ParamInfo, Texture) && Texture)
		{
			TextureMap.Add(ParamInfo.Name, Texture);
			UE_LOG(LogTemp, Log, TEXT("GetAllTexturesFromMaterial: Found texture '%s' = '%s'"), 
				*ParamInfo.Name.ToString(), *Texture->GetName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GetAllTexturesFromMaterial: Found %d textures in Material '%s'"), 
		TextureMap.Num(), *MaterialInterface->GetName());

	return TextureMap;
}

int32 UEditorToolsBPFLibrary::GetStaticMeshTriangleCount(UStaticMesh* StaticMesh, int32 LODIndex)
{
	if (!StaticMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetStaticMeshTriangleCount: StaticMesh is null"));
		return 0;
	}

	// 检查LOD索引是否有效
	if (LODIndex < 0 || LODIndex >= StaticMesh->GetNumLODs())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetStaticMeshTriangleCount: Invalid LOD index %d. Mesh '%s' has %d LODs"), 
			LODIndex, *StaticMesh->GetName(), StaticMesh->GetNumLODs());
		return 0;
	}

	// 获取渲染数据
	if (!StaticMesh->GetRenderData())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetStaticMeshTriangleCount: No render data for mesh '%s'"), *StaticMesh->GetName());
		return 0;
	}

	// 获取指定LOD的渲染数据
	const FStaticMeshLODResources& LODResources = StaticMesh->GetRenderData()->LODResources[LODIndex];
	
	// 计算总三角形数
	int32 TotalTriangles = 0;
	for (int32 SectionIndex = 0; SectionIndex < LODResources.Sections.Num(); ++SectionIndex)
	{
		const FStaticMeshSection& Section = LODResources.Sections[SectionIndex];
		TotalTriangles += Section.NumTriangles;
	}

	UE_LOG(LogTemp, Log, TEXT("GetStaticMeshTriangleCount: Mesh '%s' LOD %d has %d triangles"), 
		*StaticMesh->GetName(), LODIndex, TotalTriangles);

	return TotalTriangles;
}

TArray<FActorLightingInfo> UEditorToolsBPFLibrary::GetActorsWithInvalidLighting(UObject* WorldContextObject, bool bIncludeOnlyStatic)
{
	TArray<FActorLightingInfo> ActorsWithInvalidLighting;

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetActorsWithInvalidLighting: WorldContextObject is null"));
		return ActorsWithInvalidLighting;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetActorsWithInvalidLighting: Failed to get World from context"));
		return ActorsWithInvalidLighting;
	}

	int32 TotalActorsChecked = 0;
	int32 ActorsWithProblems = 0;

	// 遍历世界中的所有Actor（不输出详细日志）
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		// 获取Actor上的所有静态网格体组件
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

		if (StaticMeshComponents.Num() == 0)
		{
			continue;
		}

		TotalActorsChecked++;

		// 检查Actor的光照状态（内联逻辑）
	FActorLightingInfo LightingInfo;
	LightingInfo.Actor = Actor;
#if WITH_EDITOR
	LightingInfo.ActorName = Actor->GetActorLabel();
#else
	LightingInfo.ActorName = Actor->GetName();
#endif
	LightingInfo.StaticMeshComponentCount = StaticMeshComponents.Num();

	int32 ProblematicComponents = 0;
	TArray<FString> Problems;

	for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		// 检查移动性
		if (MeshComponent->Mobility == EComponentMobility::Movable || 
			MeshComponent->Mobility == EComponentMobility::Stationary)
		{
			if (LightingInfo.BuildStatus != ELightingBuildStatus::NeedRebuild && 
				LightingInfo.BuildStatus != ELightingBuildStatus::NoLightmap &&
				LightingInfo.BuildStatus != ELightingBuildStatus::InvalidSettings)
			{
				LightingInfo.BuildStatus = ELightingBuildStatus::Movable;
				Problems.Add(TEXT("组件不是静态的"));
			}
			continue;
		}

		// 检查是否有静态网格体
		UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
		if (!StaticMesh)
		{
			ProblematicComponents++;
			LightingInfo.BuildStatus = ELightingBuildStatus::InvalidSettings;
			Problems.Add(TEXT("组件没有静态网格体"));
			continue;
		}

		// 检查光照贴图分辨率
		int32 LightmapResolution = 0;
		if (MeshComponent->bOverrideLightMapRes)
		{
			LightmapResolution = MeshComponent->OverriddenLightMapRes;
		}
		else
		{
			LightmapResolution = StaticMesh->GetLightMapResolution();
		}

		if (LightmapResolution <= 0)
		{
			ProblematicComponents++;
			LightingInfo.BuildStatus = ELightingBuildStatus::InvalidSettings;
				Problems.Add(FString::Printf(TEXT("组件 '%s' 的光照贴图分辨率无效: %d"), 
					*MeshComponent->GetName(), LightmapResolution));
			continue;
		}

		// 检查光照贴图坐标索引
		if (StaticMesh->GetLightMapCoordinateIndex() < 0)
		{
			ProblematicComponents++;
			LightingInfo.BuildStatus = ELightingBuildStatus::InvalidSettings;
			Problems.Add(FString::Printf(TEXT("组件 '%s' 的光照贴图坐标索引无效"), 
				*MeshComponent->GetName()));
			continue;
		}

#if WITH_EDITOR
		// 检查是否有有效的光照贴图数据
			UWorld* ActorWorld = Actor->GetWorld();
			if (ActorWorld && ActorWorld->PersistentLevel)
		{
				UMapBuildDataRegistry* Registry = ActorWorld->PersistentLevel->MapBuildData;
			if (Registry)
			{
				FGuid MapBuildDataId;
				if (MeshComponent->LODData.Num() > 0)
				{
					MapBuildDataId = MeshComponent->LODData[0].MapBuildDataId;
				}
				
				if (MapBuildDataId.IsValid())
				{
					FMeshMapBuildData* BuildData = Registry->GetMeshBuildData(MapBuildDataId);
					if (!BuildData || !BuildData->LightMap.IsValid())
					{
						ProblematicComponents++;
						LightingInfo.BuildStatus = ELightingBuildStatus::NeedRebuild;
						Problems.Add(FString::Printf(TEXT("组件 '%s' 没有有效的光照贴图数据"), 
							*MeshComponent->GetName()));
					}
				}
				else
				{
					ProblematicComponents++;
					LightingInfo.BuildStatus = ELightingBuildStatus::NeedRebuild;
					Problems.Add(FString::Printf(TEXT("组件 '%s' 没有 MapBuildDataId"), 
						*MeshComponent->GetName()));
				}
			}
			else
			{
				ProblematicComponents++;
				LightingInfo.BuildStatus = ELightingBuildStatus::NoLightmap;
				Problems.Add(TEXT("关卡没有 MapBuildDataRegistry"));
			}
		}
#endif
	}

	LightingInfo.ProblematicComponentCount = ProblematicComponents;

	// 生成描述信息
	if (Problems.Num() > 0)
	{
		LightingInfo.Description = FString::Join(Problems, TEXT("; "));
	}
	else
	{
		LightingInfo.BuildStatus = ELightingBuildStatus::Valid;
		LightingInfo.Description = TEXT("所有组件都有有效的光照");
	}

		// 如果只包含静态的，过滤掉可移动的
		if (bIncludeOnlyStatic && LightingInfo.BuildStatus == ELightingBuildStatus::Movable)
		{
			continue;
		}

		// 如果光照状态不是有效的，添加到列表
		if (LightingInfo.BuildStatus != ELightingBuildStatus::Valid && 
			LightingInfo.BuildStatus != ELightingBuildStatus::Movable)
{
			ActorsWithInvalidLighting.Add(LightingInfo);
			ActorsWithProblems++;
	}
	}

	// 写入消息日志（风格统一为 GetHighPolyActorsInScene 的格式）
#if WITH_EDITOR
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedPtr<IMessageLogListing> MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);

	if (!MessageLogListing.IsValid())
	{
		FEditorToolsMessageLog::Initialize();
		MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	}

	if (MessageLogListing.IsValid())
	{
		// 清空之前的消息
		MessageLogListing->ClearMessages();

		// 标题
		const FString FilterModeText = bIncludeOnlyStatic ? TEXT("[仅静态组件]") : TEXT("[包含可移动组件]");
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::Format(LOCTEXT("LightingHeader", "------------------ 场景光照构建检查 {0} ------------------"), FText::FromString(FilterModeText))
			)
		);

		// 统计信息
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::Format(LOCTEXT("LightingStats", "检查了 {0} 个Actor，发现 {1} 个有光照问题的Actor"),
					FText::AsNumber(TotalActorsChecked),
					FText::AsNumber(ActorsWithProblems))
			)
		);

		// 详细列表标题
		if (ActorsWithInvalidLighting.Num() > 0)
		{
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Warning,
					LOCTEXT("LightingListHeader", "详细Actor列表（点击名称可在场景中单选定位）：")
				)
			);
		}
		
		// 计算对齐信息
		const int32 RankWidth = FString::FromInt(ActorsWithInvalidLighting.Num()).Len();
		int32 MaxNameLen = 0;
		for (const FActorLightingInfo& InfoForWidth : ActorsWithInvalidLighting)
		{
			MaxNameLen = FMath::Max(MaxNameLen, InfoForWidth.ActorName.Len());
		}

		// 详细列表
		for (int32 Rank = 0; Rank < ActorsWithInvalidLighting.Num(); ++Rank)
		{
			const FActorLightingInfo& LightingInfo = ActorsWithInvalidLighting[Rank];

			FString StatusText;
			switch (LightingInfo.BuildStatus)
			{
			case ELightingBuildStatus::NeedRebuild:	StatusText = TEXT("[需要重建]"); break;
			case ELightingBuildStatus::NoLightmap:	StatusText = TEXT("[无光照贴图]"); break;
			case ELightingBuildStatus::InvalidSettings: StatusText = TEXT("[设置无效]"); break;
			case ELightingBuildStatus::Movable:		StatusText = TEXT("[可移动]"); break;
			default:								StatusText = TEXT("[未知问题]"); break;
			}

			// 左侧序号
			FString RankStr = FString::FromInt(Rank + 1);
			// 单个数字时 # 和数字之间有空格，多个数字时没有空格
			// 对于单个数字，先添加空格，然后进行左填充；对于多个数字，直接左填充
			if ((Rank + 1) < 10)
			{
				RankStr = FString::Printf(TEXT(" %s"), *RankStr);
			}
			RankStr = RankStr.LeftPad(RankWidth);

			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				FText::FromString(FString::Printf(TEXT("#%s. %s "), *RankStr, *StatusText))
			);

			// 可点击的Actor链接（名称左对齐，右填充到最大宽度）
			FString PaddedName = LightingInfo.ActorName;
			if (PaddedName.Len() < MaxNameLen)
			{
				PaddedName += FString::ChrN(MaxNameLen - PaddedName.Len(), TEXT(' '));
			}
			if (LightingInfo.Actor)
			{
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
                Message->AddToken(FActorSelectToken::Create(LightingInfo.Actor, FText::FromString(PaddedName)));
			}
			else
			{
				Message->AddToken(FTextToken::Create(FText::FromString(PaddedName)));
			}

			// 追加详情：类型 | 位置 | 组件统计 | 问题描述
			const FString ActorClass = LightingInfo.Actor ? LightingInfo.Actor->GetClass()->GetName() : TEXT("未知");
			const FVector ActorLocation = LightingInfo.Actor ? LightingInfo.Actor->GetActorLocation() : FVector::ZeroVector;
			const FString LocationText = FString::Printf(TEXT("(X=%.0f, Y=%.0f, Z=%.0f)"), ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
			const FString DetailText = FString::Printf(
				TEXT(" [%s] %s | 组件:%d(有问题:%d)%s"),
				*ActorClass,
				*LocationText,
				LightingInfo.StaticMeshComponentCount,
				LightingInfo.ProblematicComponentCount,
				LightingInfo.Description.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" | %s"), *LightingInfo.Description)
			);
			Message->AddToken(FTextToken::Create(FText::FromString(DetailText)));

			MessageLogListing->AddMessage(Message);
		}

		// 添加结束分隔线
		{
			int32 SeparatorLen = 80;
			FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Info,
					FText::FromString(FooterSeparator)
				)
			);
		}

		// 打开消息日志窗口
		MessageLogModule.OpenMessageLog(FEditorToolsMessageLog::MessageLogName);
	}
#endif

	return ActorsWithInvalidLighting;
}



TArray<FActorMeshComplexityInfo> UEditorToolsBPFLibrary::GetHighPolyActorsInScene(UObject* WorldContextObject, int32 TriangleThreshold, bool bOnlyStaticMeshActors)
{
	TArray<FActorMeshComplexityInfo> HighPolyActors;

	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetHighPolyActorsInScene: WorldContextObject is null"));
		return HighPolyActors;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetHighPolyActorsInScene: Failed to get World from context"));
		return HighPolyActors;
	}

	int32 TotalActorsChecked = 0;
	int32 HighPolyActorsCount = 0;

	// 遍历世界中的所有Actor
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		// 如果只检查纯静态网格体Actor，过滤掉其他类型
		if (bOnlyStaticMeshActors)
		{
			// 只接受精确的AStaticMeshActor类，不包含蓝图派生类
			if (Actor->GetClass() != AStaticMeshActor::StaticClass())
			{
				continue;
			}
		}

		FActorMeshComplexityInfo ComplexityInfo;
		ComplexityInfo.Actor = Actor;
		ComplexityInfo.ActorLocation = Actor->GetActorLocation();
		ComplexityInfo.ActorClass = Actor->GetClass()->GetName();

#if WITH_EDITOR
		ComplexityInfo.ActorName = Actor->GetActorLabel();
#else
		ComplexityInfo.ActorName = Actor->GetName();
#endif

		int32 ActorLOD0Triangles = 0;
		int32 ActorLODNTriangles = 0;
		int32 LODNIndex = 0;
		int32 MaxLODCount = 0;
		bool bHasMesh = false;

		// ============ 检查静态网格体组件 ============
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

		if (StaticMeshComponents.Num() > 0)
		{
			bHasMesh = true;
			// 判断是纯静态网格体还是蓝图类
			if (Actor->GetClass() == AStaticMeshActor::StaticClass())
			{
				ComplexityInfo.MeshType = EMeshType::StaticMesh;
			}
			else
			{
				// 蓝图类或其他派生类
				ComplexityInfo.MeshType = EMeshType::StaticMesh;
			}
			ComplexityInfo.ComponentCount = StaticMeshComponents.Num();

			for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
			{
				if (!MeshComp || !MeshComp->GetStaticMesh())
				{
					continue;
				}

				UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
				if (!StaticMesh->GetRenderData())
				{
					continue;
				}

				int32 NumLODs = StaticMesh->GetNumLODs();
				if (NumLODs == 0)
				{
					continue;
				}

				// 记录最大的LOD数量（如果Actor有多个组件，取最大值）
				if (NumLODs > MaxLODCount)
				{
					MaxLODCount = NumLODs;
				}

				// 只计算LOD0和最后一个LOD
				// LOD0
				const FStaticMeshLODResources& LOD0Resources = StaticMesh->GetRenderData()->LODResources[0];
				int32 LOD0Triangles = 0;
				for (int32 SectionIndex = 0; SectionIndex < LOD0Resources.Sections.Num(); ++SectionIndex)
					{
					const FStaticMeshSection& Section = LOD0Resources.Sections[SectionIndex];
					LOD0Triangles += Section.NumTriangles;
					}
				ActorLOD0Triangles += LOD0Triangles;

				// 最后一个LOD
				if (NumLODs > 1)
				{
					LODNIndex = NumLODs - 1;
					const FStaticMeshLODResources& LODNResources = StaticMesh->GetRenderData()->LODResources[LODNIndex];
					int32 LODNTriangles = 0;
					for (int32 SectionIndex = 0; SectionIndex < LODNResources.Sections.Num(); ++SectionIndex)
					{
						const FStaticMeshSection& Section = LODNResources.Sections[SectionIndex];
						LODNTriangles += Section.NumTriangles;
					}
					ActorLODNTriangles += LODNTriangles;
					}
				}

			// 添加LOD信息到LODDetails（只添加LOD0和LOD N）
			if (ActorLOD0Triangles > 0)
			{
				ComplexityInfo.LODDetails.Add(FLODTriangleInfo(0, ActorLOD0Triangles));
				if (MaxLODCount > 1 && ActorLODNTriangles > 0)
				{
					ComplexityInfo.LODDetails.Add(FLODTriangleInfo(MaxLODCount - 1, ActorLODNTriangles));
				}
			}
			ComplexityInfo.TotalLODCount = MaxLODCount;
		}

		// ============ 检查骨骼网格体组件 ============
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

		if (SkeletalMeshComponents.Num() > 0)
		{
			bHasMesh = true;
			ComplexityInfo.MeshType = EMeshType::SkeletalMesh;
			ComplexityInfo.ComponentCount = SkeletalMeshComponents.Num();

			for (USkeletalMeshComponent* SkelMeshComp : SkeletalMeshComponents)
			{
				if (!SkelMeshComp || !SkelMeshComp->GetSkeletalMeshAsset())
				{
					continue;
				}

				USkeletalMesh* SkeletalMesh = SkelMeshComp->GetSkeletalMeshAsset();
				FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
				
				if (!RenderData)
				{
					continue;
				}

				int32 NumLODs = RenderData->LODRenderData.Num();
				if (NumLODs == 0)
				{
					continue;
				}

				// 记录最大的LOD数量（如果Actor有多个组件，取最大值）
				if (NumLODs > MaxLODCount)
				{
					MaxLODCount = NumLODs;
				}

				// 只计算LOD0和最后一个LOD
				// LOD0
				const FSkeletalMeshLODRenderData& LOD0Data = RenderData->LODRenderData[0];
				int32 LOD0Triangles = 0;
				for (int32 SectionIndex = 0; SectionIndex < LOD0Data.RenderSections.Num(); ++SectionIndex)
					{
					const FSkelMeshRenderSection& Section = LOD0Data.RenderSections[SectionIndex];
					LOD0Triangles += Section.NumTriangles;
					}
				ActorLOD0Triangles += LOD0Triangles;

				// 最后一个LOD
				if (NumLODs > 1)
				{
					LODNIndex = NumLODs - 1;
					const FSkeletalMeshLODRenderData& LODNData = RenderData->LODRenderData[LODNIndex];
					int32 LODNTriangles = 0;
					for (int32 SectionIndex = 0; SectionIndex < LODNData.RenderSections.Num(); ++SectionIndex)
					{
						const FSkelMeshRenderSection& Section = LODNData.RenderSections[SectionIndex];
						LODNTriangles += Section.NumTriangles;
					}
					ActorLODNTriangles += LODNTriangles;
					}
				}

			// 添加LOD信息到LODDetails（只添加LOD0和LOD N）
			if (ActorLOD0Triangles > 0)
			{
				ComplexityInfo.LODDetails.Add(FLODTriangleInfo(0, ActorLOD0Triangles));
				if (MaxLODCount > 1 && ActorLODNTriangles > 0)
				{
					ComplexityInfo.LODDetails.Add(FLODTriangleInfo(MaxLODCount - 1, ActorLODNTriangles));
				}
			}
			ComplexityInfo.TotalLODCount = MaxLODCount;
		}

		// 如果有网格体且LOD0超过阈值
		if (bHasMesh && ActorLOD0Triangles > 0)
		{
			TotalActorsChecked++;
			ComplexityInfo.LOD0TriangleCount = ActorLOD0Triangles;
			// 不再计算总面数，TotalTriangleCount设为0或LOD0的值
			ComplexityInfo.TotalTriangleCount = 0;

			if (ActorLOD0Triangles >= TriangleThreshold)
			{
				HighPolyActors.Add(ComplexityInfo);
				HighPolyActorsCount++;
			}
		}
	}

	// 按照LOD0三角形数量从高到低排序
	HighPolyActors.Sort([](const FActorMeshComplexityInfo& A, const FActorMeshComplexityInfo& B)
	{
		return A.LOD0TriangleCount > B.LOD0TriangleCount;
	});

#if WITH_EDITOR
	// 使用消息日志输出
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedPtr<IMessageLogListing> MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	
	if (!MessageLogListing.IsValid())
	{
		FEditorToolsMessageLog::Initialize();
		MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	}

	if (MessageLogListing.IsValid())
	{
		// 清空之前的消息
		MessageLogListing->ClearMessages();

		// 添加标题
	FString FilterModeText = bOnlyStaticMeshActors ? TEXT("[仅纯静态网格体]") : TEXT("[所有网格体]");
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::Format(LOCTEXT("HighPolyHeader", "------------------ 场景多边形统计 {0} ------------------"), FText::FromString(FilterModeText))
			)
		);

		// 添加统计信息
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::Format(LOCTEXT("HighPolyStats", "检查了 {0} 个网格体Actor，发现 {1} 个超过 {2} 三角形的Actor（基于LOD0）"), 
					FText::AsNumber(TotalActorsChecked),
					FText::AsNumber(HighPolyActorsCount),
					FText::AsNumber(TriangleThreshold))
			)
		);

		// 添加问题等级说明
	if (HighPolyActorsCount > 0)
	{
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Info,
					LOCTEXT("HighPolyLevels", "问题等级分布：极高(>5x) | 很高(>3x) | 偏高(>2x) | 超标(>1x)")
				)
			);
	}

		int32 SeparatorLen = 80;
		FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
	
		// 添加详细列表
	if (HighPolyActors.Num() > 0)
	{
			// 预计算列宽：序号宽度与名称宽度，便于对齐显示
			const int32 RankWidth = FString::FromInt(HighPolyActors.Num()).Len();
			int32 MaxNameLen = 0;
			for (const FActorMeshComplexityInfo& InfoForWidth : HighPolyActors)
			{
				MaxNameLen = FMath::Max(MaxNameLen, InfoForWidth.ActorName.Len());
			}

			SeparatorLen = FMath::Clamp(RankWidth + MaxNameLen + 50, 60, 120);
			FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));

			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Warning,
					LOCTEXT("HighPolyListHeader", "详细Actor列表（按LOD0三角形数从高到低，点击可定位）：")
				)
			);

		for (int32 Rank = 0; Rank < HighPolyActors.Num(); ++Rank)
		{
			const FActorMeshComplexityInfo& ComplexityInfo = HighPolyActors[Rank];

				// 确定网格体类型
			FString MeshTypeText;
			if (ComplexityInfo.MeshType == EMeshType::SkeletalMesh)
			{
				MeshTypeText = TEXT("骨骼网格体");
			}
				else if (ComplexityInfo.Actor && ComplexityInfo.Actor->GetClass() == AStaticMeshActor::StaticClass())
				{
					MeshTypeText = TEXT("静态网格体");
				}
				else
				{
					MeshTypeText = TEXT("蓝图类");
				}

				// 获取LOD信息
				int32 LODCount = ComplexityInfo.TotalLODCount;
				int32 LOD0Triangles = ComplexityInfo.LOD0TriangleCount;
				int32 LODNTriangles = 0;
				int32 LODNIndex = 0;
				
				// 找到最后一个LOD的信息
				if (LODCount > 1)
				{
					// 找到最大的LOD索引
					for (const FLODTriangleInfo& LODInfo : ComplexityInfo.LODDetails)
					{
						if (LODInfo.LODIndex > LODNIndex)
						{
							LODNIndex = LODInfo.LODIndex;
							LODNTriangles = LODInfo.TriangleCount;
				}
					}
				}
				else if (LODCount == 1)
				{
					// 只有一个LOD，就是LOD0
					LODNTriangles = LOD0Triangles;
					LODNIndex = 0;
				}

				// 格式化三角形数量
				FString FormattedLOD0 = FText::AsNumber(LOD0Triangles).ToString();
				FString FormattedLODN = FText::AsNumber(LODNTriangles).ToString();
				
				// 构建位置信息
				FString LocationText = FString::Printf(TEXT("(X=%.0f, Y=%.0f, Z=%.0f)"), 
					ComplexityInfo.ActorLocation.X, 
					ComplexityInfo.ActorLocation.Y, 
					ComplexityInfo.ActorLocation.Z);

				// 构建LOD信息文本
				FString LODInfoText;
				if (LODCount > 1)
			{
					LODInfoText = FString::Printf(TEXT("LOD0:%s (最大) | LOD%d:%s (最小) | 共%d个LOD层级"), 
						*FormattedLOD0, LODNIndex, *FormattedLODN, LODCount);
			}
			else
			{
					LODInfoText = FString::Printf(TEXT("LOD0:%s | 共1个LOD层级"), *FormattedLOD0);
		}

				// 计算倍数
				float Multiplier = (float)LOD0Triangles / (float)TriangleThreshold;
				
			// 根据倍数确定严重程度和颜色，直接内联使用
			// 创建可点击的消息（左侧序号按列宽对齐，序号后显示类型）
			FString RankStr = FString::FromInt(Rank + 1);
			// 单个数字时 # 和数字之间有空格，多个数字时没有空格
			// 对于单个数字，先添加空格，然后进行左填充；对于多个数字，直接左填充
			if ((Rank + 1) < 10)
	{
				RankStr = FString::Printf(TEXT(" %s"), *RankStr);
		}
			RankStr = RankStr.LeftPad(RankWidth);
			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				FText::FromString(FString::Printf(TEXT("#%s. [%s] "), *RankStr, *MeshTypeText))
			);

			// 添加可点击的Actor链接
			if (ComplexityInfo.Actor)
			{
				// 使用自定义的 FActorSelectToken，只选中Actor，不移动摄像机
				// 名称左对齐，右填充到最大宽度，保证后续列对齐
				FString PaddedName = ComplexityInfo.ActorName;
				if (PaddedName.Len() < MaxNameLen)
				{
					PaddedName += FString::ChrN(MaxNameLen - PaddedName.Len(), TEXT(' '));
			}
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
				Message->AddToken(FActorSelectToken::Create(
					ComplexityInfo.Actor,
					FText::FromString(PaddedName)
				));
			}
			else
			{
				FString PaddedName = ComplexityInfo.ActorName;
				if (PaddedName.Len() < MaxNameLen)
				{
					PaddedName += FString::ChrN(MaxNameLen - PaddedName.Len(), TEXT(' '));
				}
				Message->AddToken(FTextToken::Create(FText::FromString(PaddedName)));
			}

			// 添加详细信息（已移除类型，因为已在序号后显示）
			FString DetailText = FString::Printf(TEXT(" %s | %s | %.1fx阈值"), 
				*LocationText, *LODInfoText, Multiplier);
			Message->AddToken(FTextToken::Create(FText::FromString(DetailText)));

				MessageLogListing->AddMessage(Message);
			}
		}

		// 添加结束分隔线
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::FromString(FooterSeparator)
			)
		);

		// 打开消息日志窗口
		MessageLogModule.OpenMessageLog(FEditorToolsMessageLog::MessageLogName);
		}
#endif

	return HighPolyActors;
}

FSceneLightStatistics UEditorToolsBPFLibrary::GetSceneLightStatistics(UObject* WorldContextObject)
{
	FSceneLightStatistics Statistics;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("GetSceneLightStatistics: 无效的世界上下文"));
		return Statistics;
	}

	// 遍历场景中所有的 Actor，查找灯光组件
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		// 获取 Actor 上的所有灯光组件
		TArray<ULightComponent*> LightComponents;
		Actor->GetComponents<ULightComponent>(LightComponents);

		for (ULightComponent* LightComponent : LightComponents)
		{
			if (!LightComponent)
			{
				continue;
			}

			FLightActorInfo LightInfo;
			LightInfo.LightActor = Actor;
			LightInfo.LightName = Actor->GetActorLabel();
			LightInfo.Location = Actor->GetActorLocation();
			LightInfo.Intensity = LightComponent->Intensity;
			LightInfo.bIsEnabled = LightComponent->IsVisible();

			// 确定灯光移动性类型
			EComponentMobility::Type Mobility = LightComponent->Mobility;
			switch (Mobility)
			{
			case EComponentMobility::Static:
				LightInfo.MobilityType = ELightMobilityType::Static;
				LightInfo.MobilityText = TEXT("静态光");
				Statistics.StaticLightCount++;
				break;
			case EComponentMobility::Stationary:
				LightInfo.MobilityType = ELightMobilityType::Stationary;
				LightInfo.MobilityText = TEXT("固定光");
				Statistics.StationaryLightCount++;
				break;
			case EComponentMobility::Movable:
				LightInfo.MobilityType = ELightMobilityType::Movable;
				LightInfo.MobilityText = TEXT("动态光");
				Statistics.MovableLightCount++;
				break;
			}

			// 确定灯光类型
			if (LightComponent->IsA<UDirectionalLightComponent>())
			{
				LightInfo.LightType = ELightActorType::DirectionalLight;
				LightInfo.LightTypeText = TEXT("定向光");
			}
			else if (LightComponent->IsA<USpotLightComponent>())
			{
				LightInfo.LightType = ELightActorType::SpotLight;
				LightInfo.LightTypeText = TEXT("聚光灯");
			}
			else if (LightComponent->IsA<UPointLightComponent>())
			{
				LightInfo.LightType = ELightActorType::PointLight;
				LightInfo.LightTypeText = TEXT("点光源");
			}
			else if (LightComponent->IsA<USkyLightComponent>())
			{
				LightInfo.LightType = ELightActorType::SkyLight;
				LightInfo.LightTypeText = TEXT("天空光");
			}
			else if (LightComponent->IsA<URectLightComponent>())
			{
				LightInfo.LightType = ELightActorType::RectLight;
				LightInfo.LightTypeText = TEXT("矩形光");
			}
			else
			{
				LightInfo.LightType = ELightActorType::Other;
				LightInfo.LightTypeText = TEXT("其他光源");
			}

			Statistics.LightInfoList.Add(LightInfo);
		}
	}

	Statistics.TotalLightCount = Statistics.LightInfoList.Num();

	// 按照 动态光 -> 固定光 -> 静态光 排序
	Statistics.LightInfoList.Sort([](const FLightActorInfo& A, const FLightActorInfo& B)
	{
		// 动态光优先级最高(3)，固定光次之(2)，静态光最低(1)
		auto GetPriority = [](ELightMobilityType Type) -> int32
		{
			switch (Type)
			{
			case ELightMobilityType::Movable: return 3;
			case ELightMobilityType::Stationary: return 2;
			case ELightMobilityType::Static: return 1;
			default: return 0;
			}
		};

		int32 PriorityA = GetPriority(A.MobilityType);
		int32 PriorityB = GetPriority(B.MobilityType);

		if (PriorityA != PriorityB)
		{
			return PriorityA > PriorityB;  // 优先级高的排前面
		}

		// 同类型按名称排序
		return A.LightName < B.LightName;
	});

	// 打印到消息日志（风格与 GetHighPolyActorsInScene 保持一致）
#if WITH_EDITOR
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedPtr<IMessageLogListing> MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	
	if (!MessageLogListing.IsValid())
	{
		FEditorToolsMessageLog::Initialize();
		MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	}

	if (MessageLogListing.IsValid())
	{
		// 清空之前的消息
		MessageLogListing->ClearMessages();

		// 标题
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				LOCTEXT("SceneLightHeader", "------------------ 场景灯光统计 ------------------")
			)
		);

		// 统计信息
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::Format(LOCTEXT("SceneLightStatsTotal", "总计: {0} 个灯光"), FText::AsNumber(Statistics.TotalLightCount))
			)
		);
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::Format(LOCTEXT("SceneLightStatsDetail", "  - 动态光: {0} | 固定光: {1} | 静态光: {2}"),
					FText::AsNumber(Statistics.MovableLightCount),
					FText::AsNumber(Statistics.StationaryLightCount),
					FText::AsNumber(Statistics.StaticLightCount))
			)
		);

		// 详细列表标题
		if (Statistics.LightInfoList.Num() > 0)
		{
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Warning,
					LOCTEXT("SceneLightListHeader", "灯光详细列表（按 动态->固定->静态 排序，点击名称可单选）：")
				)
			);
		}

		// 预计算宽度
		const int32 RankWidth = FString::FromInt(Statistics.LightInfoList.Num()).Len();
		int32 MaxNameLen = 0;
		for (const FLightActorInfo& InfoForWidth : Statistics.LightInfoList)
		{
			MaxNameLen = FMath::Max(MaxNameLen, InfoForWidth.LightName.Len());
		}
		// 列表
		for (int32 Rank = 0; Rank < Statistics.LightInfoList.Num(); ++Rank)
	{
			const FLightActorInfo& Info = Statistics.LightInfoList[Rank];

			// 左侧序号
			FString RankStr = FString::FromInt(Rank + 1);
			// 单个数字时 # 和数字之间有空格，多个数字时没有空格
			// 对于单个数字，先添加空格，然后进行左填充；对于多个数字，直接左填充
			if ((Rank + 1) < 10)
			{
				RankStr = FString::Printf(TEXT(" %s"), *RankStr);
			}
			RankStr = RankStr.LeftPad(RankWidth);

			// 严重级别：动态光为警告，其余信息
			const EMessageSeverity::Type Severity = (Info.MobilityType == ELightMobilityType::Movable)
				? EMessageSeverity::Warning
				: EMessageSeverity::Info;

			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				Severity,
				FText::FromString(FString::Printf(TEXT("#%s. [%s] "), *RankStr, *Info.MobilityText))
			);

			// 可点击名称（名称左对齐，右填充到最大宽度）
			FString PaddedName = Info.LightName;
			if (PaddedName.Len() < MaxNameLen)
			{
				PaddedName += FString::ChrN(MaxNameLen - PaddedName.Len(), TEXT(' '));
			}
			if (Info.LightActor)
			{
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
				Message->AddToken(FActorSelectToken::Create(Info.LightActor, FText::FromString(PaddedName)));
			}
			else
			{
				Message->AddToken(FTextToken::Create(FText::FromString(PaddedName)));
			}

			// 详情
			const FString LocationText = FString::Printf(TEXT("(%.0f, %.0f, %.0f)"), Info.Location.X, Info.Location.Y, Info.Location.Z);
			const FString EnabledText = Info.bIsEnabled ? TEXT("✓启用") : TEXT("✗禁用");
			const FString DetailText = FString::Printf(TEXT(" - %s | %s | 强度:%.1f | %s"),
				*Info.LightTypeText, *LocationText, Info.Intensity, *EnabledText);
			Message->AddToken(FTextToken::Create(FText::FromString(DetailText)));

			MessageLogListing->AddMessage(Message);
	}

		// 添加结束分隔线
		{
			int32 SeparatorLen = 80;
			FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Info,
					FText::FromString(FooterSeparator)
				)
			);
		}

		// 打开消息日志窗口
		MessageLogModule.OpenMessageLog(FEditorToolsMessageLog::MessageLogName);
	}
#endif

	return Statistics;
}

// ==================== 未使用资源检查功能 ====================

namespace
{
	// 转换UE返回的路径格式为标准格式
	// 例如: "/All/Game/crates/MoonbackEchoes/S07" -> "/Game/crates/MoonbackEchoes/S07"
	FString NormalizeFolderPath(const FString& FolderPath)
	{
		FString NormalizedPath = FolderPath;
		
		// 移除首尾空白
		NormalizedPath.TrimStartAndEndInline();
		
		// 移除开头的引号（如果有）
		if (NormalizedPath.StartsWith(TEXT("\"")))
		{
			NormalizedPath = NormalizedPath.Mid(1);
		}
		// 移除结尾的引号（如果有）
		if (NormalizedPath.EndsWith(TEXT("\"")))
		{
			NormalizedPath = NormalizedPath.Left(NormalizedPath.Len() - 1);
		}
		
		// 转换 /All/Game/ 为 /Game/
		if (NormalizedPath.StartsWith(TEXT("/All/Game/")))
		{
			NormalizedPath = TEXT("/Game/") + NormalizedPath.Mid(10); // 10 = "/All/Game/".Len()
		}
		// 转换 /All/ 为 /Game/（如果路径不包含Game）
		else if (NormalizedPath.StartsWith(TEXT("/All/")))
		{
			NormalizedPath = TEXT("/Game/") + NormalizedPath.Mid(5); // 5 = "/All/".Len()
		}
		// 如果路径以 /Game/ 开头，保持不变
		else if (!NormalizedPath.StartsWith(TEXT("/Game/")))
		{
			// 如果路径不以 /Game/ 开头，尝试添加
			if (NormalizedPath.StartsWith(TEXT("/")))
			{
				NormalizedPath = TEXT("/Game") + NormalizedPath;
			}
			else
			{
				NormalizedPath = TEXT("/Game/") + NormalizedPath;
			}
		}
		
		// 确保路径以 / 结尾（用于搜索）
		if (!NormalizedPath.EndsWith(TEXT("/")))
		{
			NormalizedPath += TEXT("/");
		}
		
		return NormalizedPath;
	}
}

TArray<FUnusedAssetInfo> UEditorToolsBPFLibrary::FindUnusedMeshesInFolder(const TArray<FString>& FolderPaths)
{
	TArray<FUnusedAssetInfo> UnusedAssets;

#if WITH_EDITOR
	TArray<FString> EffectiveFolderPaths = FolderPaths;

	// 如果 FolderPaths 为空，从内容浏览器获取选中的文件夹
	if (EffectiveFolderPaths.Num() == 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().GetSelectedFolders(EffectiveFolderPaths);
	}

	if (EffectiveFolderPaths.Num() == 0)
	{
		UEditorToolsUtilities::LogWarningToMessageLogAndOpen(
			LOCTEXT("MeshesFolderPathEmpty", "请先在内容浏览器中选择一个或多个文件夹，然后再执行“检查未使用的模型”。")
		);
		return UnusedAssets;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 收集所有文件夹内的资源
	TArray<FAssetData> AllMeshAssets;
	FTopLevelAssetPath StaticMeshClassPath = UStaticMesh::StaticClass()->GetClassPathName();
	FTopLevelAssetPath SkeletalMeshClassPath = USkeletalMesh::StaticClass()->GetClassPathName();

	for (const FString& FolderPath : EffectiveFolderPaths)
	{
		// 转换并确保路径格式正确
		FString SearchPath = NormalizeFolderPath(FolderPath);

		// 获取文件夹内的所有资源
		TArray<FAssetData> AllAssets;
		AssetRegistry.GetAssetsByPath(FName(*SearchPath), AllAssets, true);

		// 过滤出静态网格体和骨骼网格体
		for (const FAssetData& AssetData : AllAssets)
			{
			if (AssetData.AssetClassPath == StaticMeshClassPath || AssetData.AssetClassPath == SkeletalMeshClassPath)
			{
				AllMeshAssets.Add(AssetData);
			}
		}
	}

	// 输出消息统一由消息日志承担

	// 检查每个资源是否被引用
	for (const FAssetData& AssetData : AllMeshAssets)
	{
		FName PackageName = AssetData.PackageName;
		TArray<FName> Referencers;
		AssetRegistry.GetReferencers(PackageName, Referencers);

		// 过滤掉引擎内置引用（同一文件夹内的引用也算作使用）
		bool bIsUsed = false;
		for (const FName& ReferencerPackageName : Referencers)
		{
			FString ReferencerPath = ReferencerPackageName.ToString();
			// 只要是被项目内资源引用就算使用（包括同一文件夹内的引用）
			if (ReferencerPath.StartsWith(TEXT("/Game/")))
			{
				bIsUsed = true;
				break;
			}
		}

		if (!bIsUsed)
		{
			FUnusedAssetInfo Info;
			Info.AssetPath = AssetData.PackagePath.ToString();
			Info.AssetName = AssetData.AssetName.ToString();
			Info.AssetType = AssetData.AssetClassPath.GetAssetName().ToString();
			Info.AssetObject = AssetData.GetAsset();

			UnusedAssets.Add(Info);
		}
	}

	// 显示可点击的消息日志
	FEditorToolsMessageLog::ShowUnusedAssetsReport(TEXT("模型"), EffectiveFolderPaths, UnusedAssets, AllMeshAssets.Num());
#endif

	return UnusedAssets;
}

TArray<FUnusedAssetInfo> UEditorToolsBPFLibrary::FindUnusedMaterialsInFolder(const TArray<FString>& FolderPaths)
{
	TArray<FUnusedAssetInfo> UnusedAssets;

#if WITH_EDITOR
	TArray<FString> EffectiveFolderPaths = FolderPaths;

	// 如果 FolderPaths 为空，从内容浏览器获取选中的文件夹
	if (EffectiveFolderPaths.Num() == 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().GetSelectedFolders(EffectiveFolderPaths);
	}

	if (EffectiveFolderPaths.Num() == 0)
	{
		UEditorToolsUtilities::LogWarningToMessageLogAndOpen(
			LOCTEXT("MaterialsFolderPathEmpty", "请先在内容浏览器中选择一个或多个文件夹，然后再执行“检查未使用的材质”。")
		);
		return UnusedAssets;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 收集所有文件夹内的材质
	TArray<FAssetData> FilteredMaterials;
	for (const FString& FolderPath : EffectiveFolderPaths)
	{
		// 转换并确保路径格式正确
		FString SearchPath = NormalizeFolderPath(FolderPath);

		// 获取文件夹内的所有材质
		TArray<FAssetData> MaterialAssets;
		AssetRegistry.GetAssetsByPath(FName(*SearchPath), MaterialAssets, true);
		
		// 过滤出材质资源
		for (const FAssetData& AssetData : MaterialAssets)
		{
			FString ClassName = AssetData.AssetClassPath.GetAssetName().ToString();
			if (ClassName.Contains(TEXT("Material")) && AssetData.PackagePath.ToString().StartsWith(SearchPath))
			{
				FilteredMaterials.Add(AssetData);
			}
		}
	}

	// 输出消息统一由消息日志承担

	// 检查每个资源是否被引用
	for (const FAssetData& AssetData : FilteredMaterials)
	{
		FName PackageName = AssetData.PackageName;
		TArray<FName> Referencers;
		AssetRegistry.GetReferencers(PackageName, Referencers);

		// 过滤掉引擎内置引用（同一文件夹内的引用也算作使用）
		bool bIsUsed = false;
		for (const FName& ReferencerPackageName : Referencers)
		{
			FString ReferencerPath = ReferencerPackageName.ToString();
			// 只要是被项目内资源引用就算使用（包括同一文件夹内的引用）
			if (ReferencerPath.StartsWith(TEXT("/Game/")))
			{
				bIsUsed = true;
				break;
			}
		}

		if (!bIsUsed)
		{
			FUnusedAssetInfo Info;
			Info.AssetPath = AssetData.PackagePath.ToString();
			Info.AssetName = AssetData.AssetName.ToString();
			Info.AssetType = AssetData.AssetClassPath.GetAssetName().ToString();
			Info.AssetObject = AssetData.GetAsset();

			UnusedAssets.Add(Info);
		}
	}

	// 显示可点击的消息日志
	FEditorToolsMessageLog::ShowUnusedAssetsReport(TEXT("材质"), EffectiveFolderPaths, UnusedAssets, FilteredMaterials.Num());
#endif

	return UnusedAssets;
}

TArray<FUnusedAssetInfo> UEditorToolsBPFLibrary::FindUnusedTexturesInFolder(const TArray<FString>& FolderPaths)
{
	TArray<FUnusedAssetInfo> UnusedAssets;

#if WITH_EDITOR
	TArray<FString> EffectiveFolderPaths = FolderPaths;

	// 如果 FolderPaths 为空，从内容浏览器获取选中的文件夹
	if (EffectiveFolderPaths.Num() == 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().GetSelectedFolders(EffectiveFolderPaths);
	}

	if (EffectiveFolderPaths.Num() == 0)
	{
		UEditorToolsUtilities::LogWarningToMessageLogAndOpen(
			LOCTEXT("TexturesFolderPathEmpty", "请先在内容浏览器中选择一个或多个文件夹，然后再执行“检查未使用的贴图”。")
		);
		return UnusedAssets;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 收集所有文件夹内的贴图
	TArray<FAssetData> FilteredTextures;
	for (const FString& FolderPath : EffectiveFolderPaths)
	{
		// 转换并确保路径格式正确
		FString SearchPath = NormalizeFolderPath(FolderPath);

		// 获取文件夹内的所有贴图
		TArray<FAssetData> TextureAssets;
		AssetRegistry.GetAssetsByPath(FName(*SearchPath), TextureAssets, true);
		
		// 过滤出贴图资源
		for (const FAssetData& AssetData : TextureAssets)
		{
			FString ClassName = AssetData.AssetClassPath.GetAssetName().ToString();
			if (ClassName.Contains(TEXT("Texture")) && AssetData.PackagePath.ToString().StartsWith(SearchPath))
			{
				FilteredTextures.Add(AssetData);
			}
		}
	}

	// 输出消息统一由消息日志承担

	// 检查每个资源是否被引用
	for (const FAssetData& AssetData : FilteredTextures)
	{
		FName PackageName = AssetData.PackageName;
		TArray<FName> Referencers;
		AssetRegistry.GetReferencers(PackageName, Referencers);

		// 过滤掉引擎内置引用（同一文件夹内的引用也算作使用）
		bool bIsUsed = false;
		for (const FName& ReferencerPackageName : Referencers)
		{
			FString ReferencerPath = ReferencerPackageName.ToString();
			// 只要是被项目内资源引用就算使用（包括同一文件夹内的引用）
			if (ReferencerPath.StartsWith(TEXT("/Game/")))
			{
				bIsUsed = true;
				break;
			}
		}

		if (!bIsUsed)
		{
			FUnusedAssetInfo Info;
			Info.AssetPath = AssetData.PackagePath.ToString();
			Info.AssetName = AssetData.AssetName.ToString();
			Info.AssetType = AssetData.AssetClassPath.GetAssetName().ToString();
			Info.AssetObject = AssetData.GetAsset();

			UnusedAssets.Add(Info);
		}
	}

	// 显示可点击的消息日志
	FEditorToolsMessageLog::ShowUnusedAssetsReport(TEXT("贴图"), EffectiveFolderPaths, UnusedAssets, FilteredTextures.Num());
#endif

	return UnusedAssets;
}

TArray<AStaticMeshActor*> UEditorToolsBPFLibrary::GetAllStaticMeshActorsInScene(UObject* WorldContextObject)
{
	TArray<AStaticMeshActor*> Result;

#if WITH_EDITOR
	struct FStaticMeshCollisionInfo
	{
		TWeakObjectPtr<AStaticMeshActor> Actor;
		FString ActorLabel;
		FName CollisionProfileName = NAME_None;
		bool bGenerateOverlapEvents = false;
	};

	UWorld* World = nullptr;
	if (WorldContextObject)
	{
		World = WorldContextObject->GetWorld();
	}

	if (!World && GEditor)
	{
		World = GEditor->GetEditorWorldContext().World();
	}

	if (!World)
	{
		return Result;
	}

	TArray<FStaticMeshCollisionInfo> CollisionEnabledActors;
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
		if (!IsValid(MeshComp))
		{
			continue;
		}

		const ECollisionEnabled::Type CollisionEnabled = MeshComp->GetCollisionEnabled();
		const bool bHasCollision = CollisionEnabled != ECollisionEnabled::NoCollision;

		Result.Add(Actor);

		if (bHasCollision)
		{
			FStaticMeshCollisionInfo& Info = CollisionEnabledActors.AddDefaulted_GetRef();
			Info.Actor = Actor;
			Info.ActorLabel = Actor->GetActorLabel();
			Info.CollisionProfileName = MeshComp->GetCollisionProfileName();
			Info.bGenerateOverlapEvents = MeshComp->GetGenerateOverlapEvents();
		}
	}

	TSharedPtr<IMessageLogListing> MessageLogListing = UEditorToolsUtilities::GetOrCreateMessageLogListing(true);

	if (MessageLogListing.IsValid())
	{
		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			LOCTEXT("StaticMeshCollisionHeader", "------------------ 静态网格体碰撞检查 ------------------")
		);

		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::Format(
				LOCTEXT("StaticMeshCollisionSummary", "场景静态网格体总数: {0}，处于非“无碰撞”状态的数量: {1}"),
				FText::AsNumber(Result.Num()),
				FText::AsNumber(CollisionEnabledActors.Num())
			)
		);

		if (CollisionEnabledActors.Num() > 0)
		{
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Info,
					LOCTEXT("StaticMeshCollisionListTitle", "详细列表（按 Actor 名称排序）：")
				)
			);

			CollisionEnabledActors.Sort([](const FStaticMeshCollisionInfo& Lhs, const FStaticMeshCollisionInfo& Rhs)
			{
				return Lhs.ActorLabel < Rhs.ActorLabel;
			});

			int32 Index = 1;
			for (const FStaticMeshCollisionInfo& Info : CollisionEnabledActors)
			{
				AStaticMeshActor* Actor = Info.Actor.Get();
				if (Actor)
				{
					// 单个数字时 # 和数字之间有空格，多个数字时没有空格
					int32 CurrentIndex = Index++;
					FString IndexPrefix = (CurrentIndex < 10) ? TEXT("# ") : TEXT("#");
					FString IndexText = FString::Printf(TEXT("%s%d. "), *IndexPrefix, CurrentIndex);
					
					TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
						EMessageSeverity::Info,
						FText::FromString(IndexText)
					);

					Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
					Message->AddToken(FActorSelectToken::Create(Actor, FText::FromString(Info.ActorLabel)));

					if (Info.CollisionProfileName != NAME_None)
					{
						Message->AddToken(
							FTextToken::Create(
								FText::Format(
									LOCTEXT("StaticMeshCollisionProfileToken", "[碰撞预设: {0}]"),
									FText::FromName(Info.CollisionProfileName)
								)
							)
						);
					}

					if (Info.bGenerateOverlapEvents)
					{
						Message->AddToken(FTextToken::Create(LOCTEXT("StaticMeshCollisionOverlapToken", "[重叠事件: 开启]")));
					}
					else
					{
						Message->AddToken(FTextToken::Create(LOCTEXT("StaticMeshCollisionOverlapToken_Off", "[重叠事件: 关闭]")));
					}

					Message->AddToken(FTextToken::Create(LOCTEXT("StaticMeshCollisionActionHintToken", ">> 可在“详情”中视情况调整碰撞设置")));

					MessageLogListing->AddMessage(Message);
		}
			}
		}
		else
		{
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Info,
					LOCTEXT("StaticMeshCollisionNone", "所有静态网格体的碰撞均已关闭。")
				)
			);
		}

		const int32 SeparatorLen = 80;
		const FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::FromString(FooterSeparator)
		);

		UEditorToolsUtilities::OpenMessageLogPanel();
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("GetAllStaticMeshActorsInScene can only be used in the editor."));
#endif

	return Result;
}

void UEditorToolsBPFLibrary::DisableCollisionForSelectedStaticMeshActors()
{
#if WITH_EDITOR
	struct FDisableCollisionActorInfo
	{
		TWeakObjectPtr<AStaticMeshActor> Actor;
		FString ActorLabel;
		FName PreviousProfile;
		bool bGeneratedOverlap = false;
	};

	if (!GEditor)
	{
		return;
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();
	if (!SelectedActors || SelectedActors->Num() == 0)
	{
		UEditorToolsUtilities::LogWarningToMessageLogAndOpen(
			LOCTEXT("DisableCollisionNoSelection", "请先在场景中选择至少一个静态网格体Actor，然后再执行“关闭碰撞”。")
		);
		return;
	}

	const int32 TotalSelectedActors = SelectedActors->Num();

	TArray<FDisableCollisionActorInfo> ActorsToUpdate;
	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(*It))
		{
			if (UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent())
			{
				if (MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
				{
					FDisableCollisionActorInfo& Info = ActorsToUpdate.AddDefaulted_GetRef();
					Info.Actor = StaticMeshActor;
					Info.ActorLabel = StaticMeshActor->GetActorLabel();
					Info.PreviousProfile = MeshComp->GetCollisionProfileName();
					Info.bGeneratedOverlap = MeshComp->GetGenerateOverlapEvents();
				}
			}
		}
	}

	if (ActorsToUpdate.Num() == 0)
	{
		UEditorToolsUtilities::LogWarningToMessageLogAndOpen(
			LOCTEXT("DisableCollisionNoCollisionActors", "所选静态网格体的碰撞已经处于关闭状态。")
		);
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DisableCollisionTransaction", "关闭静态网格体碰撞"));

	for (FDisableCollisionActorInfo& Info : ActorsToUpdate)
	{
		AStaticMeshActor* StaticMeshActor = Info.Actor.Get();
		if (!IsValid(StaticMeshActor))
		{
			continue;
		}

		StaticMeshActor->Modify();

		if (UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent())
		{
			MeshComp->Modify();
			MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComp->SetGenerateOverlapEvents(false);
			MeshComp->SetNotifyRigidBodyCollision(false);
			MeshComp->UpdateCollisionProfile();
			MeshComp->MarkRenderStateDirty();
			StaticMeshActor->ReregisterAllComponents();
		}
	}

	GEditor->RedrawAllViewports();

	TSharedPtr<IMessageLogListing> MessageLogListing = UEditorToolsUtilities::GetOrCreateMessageLogListing(true);

	if (MessageLogListing.IsValid())
	{
		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			LOCTEXT("DisableCollisionHeader", "------------------ 静态网格体碰撞关闭 ------------------")
		);

		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::Format(
				LOCTEXT("DisableCollisionSummary", "选择了 {0} 个 Actor，其中 {1} 个静态网格体碰撞已切换为【无碰撞】："),
				FText::AsNumber(TotalSelectedActors),
				FText::AsNumber(ActorsToUpdate.Num())
			)
		);

		int32 Index = 1;
		for (const FDisableCollisionActorInfo& Info : ActorsToUpdate)
		{
			AStaticMeshActor* StaticMeshActor = Info.Actor.Get();
			if (!IsValid(StaticMeshActor))
			{
				continue;
			}

			// 单个数字时 # 和数字之间有空格，多个数字时没有空格
			int32 CurrentIndex = Index++;
			FString IndexPrefix = (CurrentIndex < 10) ? TEXT("# ") : TEXT("#");
			FString IndexText = FString::Printf(TEXT("%s%d. "), *IndexPrefix, CurrentIndex);
			
			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				EMessageSeverity::Info,
				FText::FromString(IndexText)
			);

			Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
			Message->AddToken(FActorSelectToken::Create(StaticMeshActor, FText::FromString(StaticMeshActor->GetActorLabel())));

			if (Info.PreviousProfile != NAME_None)
			{
				Message->AddToken(
					FTextToken::Create(
						FText::Format(
							LOCTEXT("DisableCollisionProfileToken", "[原碰撞预设: {0}]"),
							FText::FromName(Info.PreviousProfile)
						)
					)
				);
			}

			if (Info.bGeneratedOverlap)
			{
				Message->AddToken(FTextToken::Create(LOCTEXT("DisableCollisionOverlapToken", "[原重叠事件: 开启]")));
			}

			Message->AddToken(FTextToken::Create(LOCTEXT("DisableCollisionResultToken", ">> 现已设置为【无碰撞】，并关闭重叠和碰撞通知")));

			MessageLogListing->AddMessage(Message);
		}

		if (ActorsToUpdate.Num() > 0)
		{
			MessageLogListing->AddMessage(
				FTokenizedMessage::Create(
					EMessageSeverity::Info,
					LOCTEXT("DisableCollisionTips", "提示：可以在“世界概览”中使用“选择”按钮快速定位，并根据需要手动恢复原碰撞。")
				)
			);
		}

		const int32 SeparatorLen = 80;
		const FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::FromString(FooterSeparator)
		);

		UEditorToolsUtilities::OpenMessageLogPanel();
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("DisableCollisionForSelectedStaticMeshActors can only be used in the editor."));
#endif
}

TArray<FTextureSizeInfo> UEditorToolsBPFLibrary::CheckTextureSizesInFolders(const TArray<FString>& FolderPaths)
{
	TArray<FTextureSizeInfo> TextureSizeInfos;

#if WITH_EDITOR
	TArray<FString> EffectiveFolderPaths = FolderPaths;

	// 如果 FolderPaths 为空，从内容浏览器获取选中的文件夹
	if (EffectiveFolderPaths.Num() == 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().GetSelectedFolders(EffectiveFolderPaths);
	}

	if (EffectiveFolderPaths.Num() == 0)
	{
		UEditorToolsUtilities::LogWarningToMessageLogAndOpen(
			LOCTEXT("TextureSizeNoFolder", "请先在内容浏览器中选择一个或多个文件夹，然后再执行“检查贴图大小”。")
		);
		return TextureSizeInfos;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 收集所有文件夹内的贴图资源
	TArray<FAssetData> AllTextureAssets;
	for (const FString& FolderPath : EffectiveFolderPaths)
	{
		FString SearchPath = NormalizeFolderPath(FolderPath);
		TArray<FAssetData> TextureAssets;
		AssetRegistry.GetAssetsByPath(FName(*SearchPath), TextureAssets, true);

		// 过滤出贴图资源
		for (const FAssetData& AssetData : TextureAssets)
		{
			FString ClassName = AssetData.AssetClassPath.GetAssetName().ToString();
			if (ClassName.Contains(TEXT("Texture")) && AssetData.PackagePath.ToString().StartsWith(SearchPath))
			{
				AllTextureAssets.Add(AssetData);
			}
		}
	}

	// 获取每个贴图的分辨率信息
	for (const FAssetData& AssetData : AllTextureAssets)
	{
		UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset());
		if (Texture)
		{
			FTextureSizeInfo Info;
			Info.TexturePath = AssetData.PackagePath.ToString();
			Info.TextureName = AssetData.AssetName.ToString();
			Info.Width = Texture->GetSizeX();
			Info.Height = Texture->GetSizeY();
			Info.MaxSize = FMath::Max(Info.Width, Info.Height);
			Info.TextureObject = Texture;

			TextureSizeInfos.Add(Info);
		}
	}

	// 按最大尺寸从大到小排序
	TextureSizeInfos.Sort([](const FTextureSizeInfo& A, const FTextureSizeInfo& B)
	{
		return A.MaxSize > B.MaxSize;
	});

	// 显示消息日志
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedPtr<IMessageLogListing> MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);

	if (!MessageLogListing.IsValid())
	{
		FEditorToolsMessageLog::Initialize();
		MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	}

	if (!MessageLogListing.IsValid())
	{
		return TextureSizeInfos;
	}

	// 清空之前的消息
	MessageLogListing->ClearMessages();

	// 添加标题
	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			LOCTEXT("TextureSizeHeader", "------------------ 检查贴图大小 ------------------")
		)
	);

	// 添加文件夹路径信息
	FString FolderPathsText;
	if (EffectiveFolderPaths.Num() == 1)
	{
		FolderPathsText = EffectiveFolderPaths[0];
	}
	else
	{
		FolderPathsText = FString::Printf(TEXT("%d个文件夹"), EffectiveFolderPaths.Num());
	}

	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::Format(LOCTEXT("TextureSizeFolderPath", "文件夹路径: {0}"), FText::FromString(FolderPathsText))
		)
	);

	// 添加统计信息
	int32 LargeTextureCount = 0;
	for (const FTextureSizeInfo& Info : TextureSizeInfos)
	{
		if (Info.MaxSize > 1024)
		{
			LargeTextureCount++;
		}
	}

	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::Format(LOCTEXT("TextureSizeStats", "找到 {0} 个贴图资源，其中 {1} 个贴图的最大尺寸超过 1024"), 
				FText::AsNumber(TextureSizeInfos.Num()),
				FText::AsNumber(LargeTextureCount))
		)
	);

	// 添加详细列表
	if (TextureSizeInfos.Num() > 0)
	{
		// 计算对齐信息
		const int32 RankWidth = FString::FromInt(TextureSizeInfos.Num()).Len();
		int32 MaxNameLen = 0;
		for (const FTextureSizeInfo& InfoForWidth : TextureSizeInfos)
		{
			MaxNameLen = FMath::Max(MaxNameLen, InfoForWidth.TextureName.Len());
		}

		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				LOCTEXT("TextureSizeListHeader", "详细贴图列表（按分辨率从大到小排序，点击可定位）：")
			)
		);

		for (int32 i = 0; i < TextureSizeInfos.Num(); ++i)
		{
			const FTextureSizeInfo& Info = TextureSizeInfos[i];

			// 左侧序号
			FString RankStr = FString::FromInt(i + 1);
			// 单个数字时 # 和数字之间有空格，多个数字时没有空格
			// 对于单个数字，先添加空格，然后进行左填充；对于多个数字，直接左填充
			if ((i + 1) < 10)
			{
				RankStr = FString::Printf(TEXT(" %s"), *RankStr);
			}
			RankStr = RankStr.LeftPad(RankWidth);

			// 判断是否大于1024，使用警告级别
			const bool bIsLarge = Info.MaxSize > 1024;
			const EMessageSeverity::Type Severity = bIsLarge ? EMessageSeverity::Warning : EMessageSeverity::Info;

			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				Severity,
				FText::FromString(FString::Printf(TEXT("#%s. [贴图] "), *RankStr))
			);

			// 添加可点击的贴图链接（名称左对齐，右填充到最大宽度）
			FString PaddedName = Info.TextureName;
			if (PaddedName.Len() < MaxNameLen)
			{
				PaddedName += FString::ChrN(MaxNameLen - PaddedName.Len(), TEXT(' '));
			}

			if (Info.TextureObject)
			{
				// 手动添加放大镜图标以保持一致性
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
				Message->AddToken(FAssetObjectToken::Create(Info.TextureObject, FText::FromString(PaddedName)));
		}
			else
			{
				// 如果无法加载贴图，使用文本显示
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
				Message->AddToken(FTextToken::Create(FText::FromString(PaddedName)));
			}

			// 添加分辨率信息（用[]框起来）
			FString SizeText = FString::Printf(TEXT(" [%dx%d]"), Info.Width, Info.Height);
			Message->AddToken(FTextToken::Create(FText::FromString(SizeText)));

			// 添加路径信息
			FString FullAssetPath = Info.TexturePath + TEXT("/") + Info.TextureName;
			Message->AddToken(FTextToken::Create(FText::Format(LOCTEXT("TexturePath", " ({0})"), FText::FromString(FullAssetPath))));

			MessageLogListing->AddMessage(Message);
		}
	}

	// 添加结束分隔线
	const int32 SeparatorLen = 80;
	const FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::FromString(FooterSeparator)
		)
	);

	// 打开消息日志窗口
	MessageLogModule.OpenMessageLog(FEditorToolsMessageLog::MessageLogName);
#endif

	return TextureSizeInfos;
}

#undef LOCTEXT_NAMESPACE
