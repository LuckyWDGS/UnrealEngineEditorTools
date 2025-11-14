// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "TextureSizeTypes.generated.h"

/**
 * 贴图大小信息结构体
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FTextureSizeInfo
{
	GENERATED_BODY()

	// 贴图路径
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size Info")
	FString TexturePath;

	// 贴图名称
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size Info")
	FString TextureName;

	// 贴图宽度
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size Info")
	int32 Width;

	// 贴图高度
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size Info")
	int32 Height;

	// 贴图对象引用（如果已加载）
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size Info")
	UTexture2D* TextureObject;

	// 最大尺寸（Width 和 Height 中的较大值）
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size Info")
	int32 MaxSize;

	FTextureSizeInfo()
		: TexturePath(TEXT(""))
		, TextureName(TEXT(""))
		, Width(0)
		, Height(0)
		, TextureObject(nullptr)
		, MaxSize(0)
	{
	}
};

