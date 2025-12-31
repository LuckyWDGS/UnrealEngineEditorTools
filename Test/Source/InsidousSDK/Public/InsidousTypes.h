// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InsidousTypes.generated.h"

USTRUCT(BlueprintType)
struct FSequenceInfo {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	bool hasSequence = false;
	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	bool levelFinished = false;
	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	int32 TArea = 0;
	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	int32 second = 0;
	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	bool isVO = false;
};

// 用于载具被启用时 OnLogging 调用时传递玩家给载具
USTRUCT(BlueprintType)
struct FPlayerParam {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	APawn* player = nullptr;
};