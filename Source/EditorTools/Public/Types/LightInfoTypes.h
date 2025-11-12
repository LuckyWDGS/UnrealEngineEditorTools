// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightInfoTypes.generated.h"

/** 灯光移动性类型枚举 */
UENUM(BlueprintType)
enum class ELightMobilityType : uint8
{
	/** 静态光 */
	Static UMETA(DisplayName = "静态光"),
	
	/** 固定光 */
	Stationary UMETA(DisplayName = "固定光"),
	
	/** 动态光 */
	Movable UMETA(DisplayName = "动态光")
};

/** 灯光类型枚举 */
UENUM(BlueprintType)
enum class ELightActorType : uint8
{
	/** 定向光 */
	DirectionalLight UMETA(DisplayName = "定向光"),
	
	/** 点光源 */
	PointLight UMETA(DisplayName = "点光源"),
	
	/** 聚光灯 */
	SpotLight UMETA(DisplayName = "聚光灯"),
	
	/** 天空光 */
	SkyLight UMETA(DisplayName = "天空光"),
	
	/** 矩形光 */
	RectLight UMETA(DisplayName = "矩形光"),
	
	/** 其他光源 */
	Other UMETA(DisplayName = "其他光源")
};

/** 单个灯光的详细信息 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FLightActorInfo
{
	GENERATED_BODY()

	/** 灯光Actor引用 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	AActor* LightActor = nullptr;

	/** 灯光名称 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	FString LightName;

	/** 灯光在世界中的位置 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	FVector Location = FVector::ZeroVector;

	/** 灯光移动性类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	ELightMobilityType MobilityType = ELightMobilityType::Static;

	/** 灯光Actor类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	ELightActorType LightType = ELightActorType::Other;

	/** 灯光移动性文本描述 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	FString MobilityText;

	/** 灯光类型文本描述 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	FString LightTypeText;

	/** 灯光强度 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	float Intensity = 0.0f;

	/** 是否启用 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Info")
	bool bIsEnabled = true;
};

/** 场景灯光统计信息 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FSceneLightStatistics
{
	GENERATED_BODY()

	/** 静态光数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Statistics")
	int32 StaticLightCount = 0;

	/** 固定光数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Statistics")
	int32 StationaryLightCount = 0;

	/** 动态光数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Statistics")
	int32 MovableLightCount = 0;

	/** 总灯光数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Light Statistics")
	int32 TotalLightCount = 0;

	/** 所有灯光的详细信息列表（按 动态->固定->静态 排序） */
	UPROPERTY(BlueprintReadOnly, Category = "Light Statistics")
	TArray<FLightActorInfo> LightInfoList;
};

