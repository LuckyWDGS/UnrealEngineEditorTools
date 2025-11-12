// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "UnusedAssetTypes.generated.h"

/**
 * 未使用的资源信息结构体
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FUnusedAssetInfo
{
	GENERATED_BODY()

	// 资源路径
	UPROPERTY(BlueprintReadOnly, Category = "Unused Asset Info")
	FString AssetPath;

	// 资源名称
	UPROPERTY(BlueprintReadOnly, Category = "Unused Asset Info")
	FString AssetName;

	// 资源类型（StaticMesh, SkeletalMesh, Material, Texture等）
	UPROPERTY(BlueprintReadOnly, Category = "Unused Asset Info")
	FString AssetType;

	// 资源对象引用（如果已加载）
	UPROPERTY(BlueprintReadOnly, Category = "Unused Asset Info")
	UObject* AssetObject;

	FUnusedAssetInfo()
		: AssetPath(TEXT(""))
		, AssetName(TEXT(""))
		, AssetType(TEXT(""))
		, AssetObject(nullptr)
	{
	}
};

