// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawCallTypes.generated.h"

/**
 * Actor DrawCall信息结构体
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FActorDrawCallInfo
{
	GENERATED_BODY()

	// Actor引用
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	AActor* Actor;

	// Actor名称（关卡大纲中的名称）
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	FString ActorName;

	// Actor类型
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	FString ActorClass;

	// Actor位置
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	FVector ActorLocation;

	// 网格体类型（静态网格体、蓝图类、骨骼网格体）
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	FString MeshTypeText;

	// 总DrawCall数量
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	int32 TotalDrawCalls;

	// LOD0的DrawCall数量
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	int32 LOD0DrawCalls;

	// 组件数量
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	int32 ComponentCount;

	// 材质槽总数
	UPROPERTY(BlueprintReadOnly, Category = "DrawCall Info")
	int32 TotalMaterialSlots;

	FActorDrawCallInfo()
		: Actor(nullptr)
		, ActorName(TEXT(""))
		, ActorClass(TEXT(""))
		, ActorLocation(FVector::ZeroVector)
		, MeshTypeText(TEXT(""))
		, TotalDrawCalls(0)
		, LOD0DrawCalls(0)
		, ComponentCount(0)
		, TotalMaterialSlots(0)
	{
	}
};


