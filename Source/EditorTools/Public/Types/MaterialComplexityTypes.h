// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "MaterialComplexityTypes.generated.h"

/**
 * 材质复杂度信息结构体
 * 用于存储和传递材质的性能相关信息
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FMaterialComplexityInfo
{
	GENERATED_BODY()

	// 顶点着色器指令数（估算值）
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	int32 VertexShaderInstructionCount;

	// 像素着色器指令数（估算值）
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	int32 PixelShaderInstructionCount;

	// 纹理采样器数量
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	int32 TextureSampleCount;

	// 是否使用世界位置偏移
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	bool bUsesWorldPositionOffset;

	// 是否使用像素深度偏移
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	bool bUsesPixelDepthOffset;

	// 混合模式（如：Opaque、Translucent、Masked等）
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	FString BlendMode;

	// 着色模型（如：DefaultLit、Unlit、Subsurface等）
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	FString ShadingModel;

	// 是否是双面材质
	UPROPERTY(BlueprintReadOnly, Category = "Material Complexity")
	bool bIsTwoSided;

	// 构造函数 - 初始化默认值
	FMaterialComplexityInfo()
		: VertexShaderInstructionCount(0)
		, PixelShaderInstructionCount(0)
		, TextureSampleCount(0)
		, bUsesWorldPositionOffset(false)
		, bUsesPixelDepthOffset(false)
		, BlendMode(TEXT("Opaque"))
		, ShadingModel(TEXT("DefaultLit"))
		, bIsTwoSided(false)
	{
	}
};


