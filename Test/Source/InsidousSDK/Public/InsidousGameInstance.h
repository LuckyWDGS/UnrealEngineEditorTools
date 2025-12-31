// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FlowControlInterface.h"
#include "JsonLibrary/Public/JsonLibrary.h"
#include "Engine/TimerHandle.h"
#include "InsidousGameInstance.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FMeshVertices
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousSDK")
	TArray<FVector> Points;
};

class USocketObject;
class UMaterialInstance;

/**
 *
 */
UCLASS()
class INSIDOUSSDK_API UInsidousGameInstance : public UGameInstance, public IFlowControlInterface
{
	GENERATED_BODY()

	UInsidousGameInstance();

public:

	FTimerHandle ReconnectTimerHandle;
	FTimerHandle CalibrateTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString DeviceSN;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	int32 DeviceID;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString FileServerHost;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString contentName;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString AppName;

	// 玩家名称
	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString PlayerName;

	// 玩家在小组中的序号
	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	int32 InTeamIndex;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString contentVer;

	// 开始体验时 设置的启动参数，由客户端自行解析和处理参数
	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	FString StartParams;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	float distance_cm;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	float teammate_distance_cm;

	// 最终需要播放的，可能比部署的内容少
	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	TArray<FPathLevelInfo> PathLevelInfos;

	// 部署的内容，设置完成后不可修改
	UPROPERTY(BlueprintReadOnly, Category = "TempStorage")
	TArray<FPathLevelInfo> DeployedPathLevelInfos;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	TArray<FInsidousMediaCam> InsidousMediaCameras;

	UPROPERTY(BlueprintReadWrite, Category = "TempStorage")
	int32 CurrentPathIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InsidousSDK")
	UMaterialInstance *ArenaMeshMaterial;

	UPROPERTY(BlueprintReadOnly, Category = "InsidousSDK")
	USocketObject *SocketObject;

	int32 TargetSeqSecond;

	TMap<int32, float> SeletalMeshScale;

	UPROPERTY(BlueprintReadOnly, Category = "TempStorage")
	bool IsConnected;

	TArray<FMeshVertices> MeshVertices;

public:
	UFUNCTION(BlueprintCallable, Category = "InsidousSDK")
	void RegistSocketObject(USocketObject *obj);

	// 蓝图重载事件，因为需要调用pak插件相关的功能
	UFUNCTION(BlueprintImplementableEvent, Category = "InsidousSDK")
	void OnFindPlayerBP();

	UFUNCTION(BlueprintCallable, Category = "InsidousSDK")
	void OnGetPlayerInfo(bool isLeader, int32 targetLevelIndex, bool needJumpToLevel, int32 targetSeqSec, bool needJumpToSeqSecond, int32 currentLevelIndex, int32 currentSeqSecond);

	virtual void Init() override;

	UFUNCTION()
	void OnKickout(FJsonLibraryObject JsonObject);
	UFUNCTION()
	void OnGetServerMsg(FJsonLibraryObject JsonObject);
	UFUNCTION()
	void OnDisconnect();
	UFUNCTION()
	void OnReconnect();

	// IFlowControlInterface 实现
	UFUNCTION(BlueprintNativeEvent, Category = "FlowControlInterface")
	void LevelStart();

	UFUNCTION(BlueprintNativeEvent, Category = "FlowControlInterface")
	FGetCurrentPathResult GetCurrentPath();
	// 蓝图需要调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
	float FindOtherPlayerScale(int32 userID);
	// 蓝图需要调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
	void AddOtherPlayerScale(int32 userID, float scale);
	// 蓝图需要调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
	FOtherPlayerViewDistance GetOtherPlayerViewDis();
	// 蓝图需要调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
	int32 GetTargetSeqSecond();

	// 提供默认实现，蓝图可覆盖
	UFUNCTION(BlueprintNativeEvent, Category = "FlowControlInterface")
	void OnStartPlay();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
	FString GetPlayerName();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FlowControlInterface")
	int32 GetInTeamIndex();

	// 重置当前目标时序信息
	// CurrentPathIndex = -1;
	// TargetSeqSecond = 0;
	UFUNCTION(BlueprintCallable, Category = "FlowControlInterface")
	void ResetCurrentSeqInfo();
private:

	void LoadDeviceSN();

	void OpenMapServerConnection();
	void ParseArena(FString data);
	void ParsePathInfo(FString data);
	void OnFindPlayer(FString data);
	void OnStopPlay();

	void CreateArenaAndAttach(USceneComponent *attachTo, AActor *target);
	void CreateInsidousMediaCameraAndAttach(USceneComponent* attachTo, AActor* target);
	void AsyncLoadNextLevel();
};
