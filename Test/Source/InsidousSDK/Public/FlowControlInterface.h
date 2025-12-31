// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InsidousTypes.h"
#include "FlowControlInterface.generated.h"


USTRUCT(BlueprintType)
struct FPathLevelInfo {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	FString path;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	FVector loc;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	float rot_z;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	FVector start_loc;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	float start_rot;

	// default start from 0
	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	int32 start_second = 0;

	// default play to end
	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	int32 end_second = TNumericLimits<int32>::Max();

};

USTRUCT(BlueprintType)
struct FInsidousMediaCam {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	int32 id;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	FVector loc;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	FRotator rot;
};


USTRUCT(BlueprintType)
struct FOtherPlayerViewDistance {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	float distance_cm = 0;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	float teammate_distance_cm = 0;
};

USTRUCT(BlueprintType)
struct FGetCurrentPathResult {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	bool is_success = false;

	UPROPERTY(BlueprintReadWrite, Category = "FlowControl")
	FPathLevelInfo path;
};

UINTERFACE(BlueprintType)
class UFlowControlInterface : public UInterface
{
	GENERATED_BODY()
};


class INSIDOUSSDK_API IFlowControlInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 蓝图实现
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		void JumpToLevel(int32 levelIndex);

	// 蓝图实现
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		void TheEnd();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		void LevelStart();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		FGetCurrentPathResult GetCurrentPath();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		int32 GetTargetSeqSecond();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		void SendSeqInfo(FSequenceInfo info);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		float FindOtherPlayerScale(int32 userID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		void AddOtherPlayerScale(int32 userID, float scale);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		FOtherPlayerViewDistance GetOtherPlayerViewDis();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		FString GetPlayerName();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
		int32 GetInTeamIndex();
	
};
