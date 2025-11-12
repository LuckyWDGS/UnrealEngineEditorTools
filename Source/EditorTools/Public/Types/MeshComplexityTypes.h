// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshComplexityTypes.generated.h"

/**
 * 网格体类型枚举
 */
UENUM(BlueprintType)
enum class EMeshType : uint8
{
	StaticMesh UMETA(DisplayName = "Static Mesh"),
	SkeletalMesh UMETA(DisplayName = "Skeletal Mesh")
};

/**
 * LOD级别三角形信息
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FLODTriangleInfo
{
	GENERATED_BODY()

	// LOD索引
	UPROPERTY(BlueprintReadOnly, Category = "LOD Info")
	int32 LODIndex;

	// 该LOD的三角形数量
	UPROPERTY(BlueprintReadOnly, Category = "LOD Info")
	int32 TriangleCount;

	FLODTriangleInfo()
		: LODIndex(0)
		, TriangleCount(0)
	{
	}

	FLODTriangleInfo(int32 InLODIndex, int32 InTriangleCount)
		: LODIndex(InLODIndex)
		, TriangleCount(InTriangleCount)
	{
	}
};

/**
 * Actor网格体复杂度信息结构体
 */
USTRUCT(BlueprintType)
struct EDITORTOOLS_API FActorMeshComplexityInfo
{
	GENERATED_BODY()

	// Actor引用
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	AActor* Actor;

	// Actor名称（关卡大纲中的名称）
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	FString ActorName;

	// Actor类型
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	FString ActorClass;

	// Actor位置
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	FVector ActorLocation;

	// 网格体类型
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	EMeshType MeshType;

	// 总三角形数量（所有LOD）
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	int32 TotalTriangleCount;

	// LOD 0的三角形数量
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	int32 LOD0TriangleCount;

	// 所有LOD的详细信息（只包含LOD0和LOD N）
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	TArray<FLODTriangleInfo> LODDetails;

	// 总LOD层级数量
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	int32 TotalLODCount;

	// 组件数量
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Complexity")
	int32 ComponentCount;

	FActorMeshComplexityInfo()
		: Actor(nullptr)
		, ActorName(TEXT(""))
		, ActorClass(TEXT(""))
		, ActorLocation(FVector::ZeroVector)
		, MeshType(EMeshType::StaticMesh)
		, TotalTriangleCount(0)
		, LOD0TriangleCount(0)
		, TotalLODCount(0)
		, ComponentCount(0)
	{
	}
};


