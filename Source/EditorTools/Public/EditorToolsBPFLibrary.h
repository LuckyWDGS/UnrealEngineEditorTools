// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Types/MaterialComplexityTypes.h"
#include "Types/LightingBuildTypes.h"
#include "Types/MeshComplexityTypes.h"
#include "Types/DrawCallTypes.h"
#include "Types/LightInfoTypes.h"
#include "Types/UnusedAssetTypes.h"
#include "Types/TextureSizeTypes.h"

#include "EditorToolsBPFLibrary.generated.h"

UCLASS()
class EDITORTOOLS_API UEditorToolsBPFLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ==================== 材质相关功能 ====================
	
	//获取静态网格体使用的全部材质
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material")
	static TArray<UMaterialInterface*> GetStaticMeshMaterials(UStaticMesh* StaticMesh);

	//从基础材质创建材质实例
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material")
	static UMaterialInstanceConstant* CreateMaterialInstanceFromBase(UMaterialInterface* BaseMaterial, const FString& SavePath);

	//设置材质实例的纹理参数
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material")
	static void SetMICTexture(UMaterialInterface* MaterialInterface, FName ParamName, UTexture* NewTexture);

	//从材质中获取指定参数名的纹理
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material")
	static UTexture* GetTextureFromMaterial(UMaterialInterface* MaterialInterface, FName ParameterName);

	//获取材质中所有纹理参数的名称
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material")
	static TArray<FName> GetAllTextureParameterNames(UMaterialInterface* MaterialInterface);

	//获取材质中所有纹理参数（名称和纹理的映射）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material")
	static TMap<FName, UTexture*> GetAllTexturesFromMaterial(UMaterialInterface* MaterialInterface);

	// ==================== 材质组件操作 ====================
	
	//设置场景中静态网格体组件的材质（仅影响当前实例，不修改资产）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material Component")
	static void SetStaticMeshComponentMaterial(UStaticMeshComponent* MeshComponent, int32 MaterialIndex, UMaterialInterface* Material);

	//获取场景中静态网格体组件的材质
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material Component")
	static UMaterialInterface* GetStaticMeshComponentMaterial(UStaticMeshComponent* MeshComponent, int32 MaterialIndex);

	//获取场景中静态网格体组件的所有材质
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material Component")
	static TArray<UMaterialInterface*> GetStaticMeshComponentMaterials(UStaticMeshComponent* MeshComponent);

	//设置Actor上所有静态网格体组件的指定材质槽
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Material Component")
	static void SetActorStaticMeshesMaterial(AActor* Actor, int32 MaterialIndex, UMaterialInterface* Material);

	// ==================== 纹理资源 ====================
	
	//从路径和文件名加载纹理资源
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Texture")
	static UTexture* LoadTextureFromPath(const FString& AssetPath, const FString& AssetName);

	// ==================== 网格体复杂度分析 ====================
	
	//获取静态网格体的三角形面数（单个网格体，指定LOD）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Mesh Complexity")
	static int32 GetStaticMeshTriangleCount(UStaticMesh* StaticMesh, int32 LODIndex = 0);

	//获取场景中所有超过指定三角面数的网格体Actor（显示LOD0和LOD N，静态和骨骼网格体）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Mesh Complexity", meta = (WorldContext = "WorldContextObject"))
	static TArray<FActorMeshComplexityInfo> GetHighPolyActorsInScene(UObject* WorldContextObject, int32 TriangleThreshold = 100000, bool bOnlyStaticMeshActors = true);

	// ==================== 静态网格体管理 ====================

	// 获取当前场景中所有的静态网格体Actor，并在消息日志中列出开启碰撞的Actor（带定位功能）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Static Mesh", meta = (WorldContext = "WorldContextObject"))
	static TArray<AStaticMeshActor*> GetAllStaticMeshActorsInScene(UObject* WorldContextObject);

	// 关闭当前所选静态网格体Actor的碰撞
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Static Mesh")
	static void DisableCollisionForSelectedStaticMeshActors();

	// ==================== 光照构建 ====================
	
	//获取场景中所有需要重新构建光照的Actor（可选在屏幕上用红色显示）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Lighting Build", meta = (WorldContext = "WorldContextObject"))
	static TArray<FActorLightingInfo> GetActorsWithInvalidLighting(UObject* WorldContextObject, bool bIncludeOnlyStatic = true);

	// ==================== 灯光统计 ====================
	
	//获取场景中所有灯光的统计信息（按 动态光->固定光->静态光 排序）
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Light Statistics", meta = (WorldContext = "WorldContextObject"))
	static FSceneLightStatistics GetSceneLightStatistics(UObject* WorldContextObject);
	
	// ==================== 未使用资源检查功能 ====================
	
	//检查指定文件夹内未使用的模型（静态网格体和骨骼网格体）
	//如果FolderPaths为空，则从内容浏览器获取选中的文件夹
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Unused Assets")
	static TArray<FUnusedAssetInfo> FindUnusedMeshesInFolder(const TArray<FString>& FolderPaths);
	
	//检查指定文件夹内未使用的材质
	//如果FolderPaths为空，则从内容浏览器获取选中的文件夹
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Unused Assets")
	static TArray<FUnusedAssetInfo> FindUnusedMaterialsInFolder(const TArray<FString>& FolderPaths);
	
	//检查指定文件夹内未使用的贴图
	//如果FolderPaths为空，则从内容浏览器获取选中的文件夹
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Unused Assets")
	static TArray<FUnusedAssetInfo> FindUnusedTexturesInFolder(const TArray<FString>& FolderPaths);

	// ==================== 贴图大小检查功能 ====================
	
	//检查指定文件夹内使用的贴图大小（根据分辨率从大到小排序，大于1024的用黄色感叹号标注）
	//如果FolderPaths为空，则从内容浏览器获取选中的文件夹
	UFUNCTION(BlueprintCallable, Category = "Editor Tools BP Library|Texture Size")
	static TArray<FTextureSizeInfo> CheckTextureSizesInFolders(const TArray<FString>& FolderPaths);

	
	
};