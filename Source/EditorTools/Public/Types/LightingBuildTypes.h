// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightingBuildTypes.generated.h"

/**
 * 光照构建状态枚举
 */
UENUM(BlueprintType)
enum class ELightingBuildStatus : uint8
{
	Valid UMETA(DisplayName = "Valid"),							// 光照有效
	NeedRebuild UMETA(DisplayName = "Need Rebuild"),			// 需要重新构建
	NoLightmap UMETA(DisplayName = "No Lightmap"),				// 没有光照贴图
	InvalidSettings UMETA(DisplayName = "Invalid Settings"),	// 设置无效
	Movable UMETA(DisplayName = "Movable")						// 可移动的（不需要烘焙）
};

/**
 * Actor光照构建信息结构体
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FActorLightingInfo
{
	GENERATED_BODY()

	// Actor引用
	UPROPERTY(BlueprintReadOnly, Category = "Lighting Info")
	AActor* Actor;

	// Actor名称
	UPROPERTY(BlueprintReadOnly, Category = "Lighting Info")
	FString ActorName;

	// 光照构建状态
	UPROPERTY(BlueprintReadOnly, Category = "Lighting Info")
	ELightingBuildStatus BuildStatus;

	// 问题描述
	UPROPERTY(BlueprintReadOnly, Category = "Lighting Info")
	FString Description;

	// 静态网格体组件数量
	UPROPERTY(BlueprintReadOnly, Category = "Lighting Info")
	int32 StaticMeshComponentCount;

	// 有问题的组件数量
	UPROPERTY(BlueprintReadOnly, Category = "Lighting Info")
	int32 ProblematicComponentCount;

	FActorLightingInfo()
		: Actor(nullptr)
		, ActorName(TEXT(""))
		, BuildStatus(ELightingBuildStatus::Valid)
		, Description(TEXT(""))
		, StaticMeshComponentCount(0)
		, ProblematicComponentCount(0)
	{
	}
};


