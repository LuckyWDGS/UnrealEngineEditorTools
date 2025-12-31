// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InsidousTypes.h"
#include "InsidousPlayerInterface.generated.h"

USTRUCT(BlueprintType)
struct FPlayerPos {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 Loc_X = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 Loc_Y = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 Loc_Z = 0;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 Rot_Z = 0;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 LeftHand_Loc_X = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 LeftHand_Loc_Y = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 LeftHand_Loc_Z = 0;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 LeftHand_Rot_X = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 LeftHand_Rot_Y = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 LeftHand_Rot_Z = 0;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 RightHand_Loc_X = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 RightHand_Loc_Y = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 RightHand_Loc_Z = 0;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 RightHand_Rot_X = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 RightHand_Rot_Y = 0;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 RightHand_Rot_Z = 0;
};


USTRUCT(BlueprintType)
struct FOtherPlayerData {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	FString name;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	int32 id;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	FVector player_pos;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	FVector lefthand_pos;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	FRotator lefthand_rot;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	FVector righthand_pos;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	FRotator righthand_rot;

	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	uint8 gender;
	UPROPERTY(BlueprintReadWrite, Category = "InsidousPlayer")
	float Player_rot_z;
};


// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UInsidousPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class USceneComponent;

class INSIDOUSSDK_API IInsidousPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		FPlayerPos GetPlayerPos();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		void OnGetOtherPlayerInfo(const TArray<FOtherPlayerData>& info);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		void SetMainSequenceTime(int32 sec, bool isTeamLeader);

	// 强制跳转到指定的 sequence 时序
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		void ForceSetMainSequenceTime(int32 sec);
	
	// 获取玩家相机相对于 玩家Pawn 的位置，用于计算与红蓝框的相对位置，因为红蓝框和 Levelmanager 与玩家Pawn 的位置一致
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		FVector GetCameraRelativeLoction();

	// 获取玩家相机相对于 玩家Pawn 的旋转，用于计算与红蓝框的相对位置，因为红蓝框和 Levelmanager 与玩家Pawn 的位置一致
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		FRotator GetCameraRelativeRotation();

	// 获取玩家相机相对于现实世界原点的位置
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
	FVector GetCameraRelativeLoctionToCameraRoot();

	// 获取玩家相机相对于现实世界原点的旋转
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
	FRotator GetCameraRelativeRotationToCameraRoot();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		USceneComponent* GetCameraRoot();

	// 为了不在代码层过多依赖其他对象，因此增加该接口获得相关的数据
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		void SetPlayerStatus(int32 TArea, float VOTime);

	// 当系统出现异常时，比如玩家掉线或者追踪丢失
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
		void OnInsidousSysError(const FString& msg);

};
