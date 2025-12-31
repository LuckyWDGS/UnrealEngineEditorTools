// Fill out your copyright notice in the Description page of Project Settings.

#include "InsidousGameInstance.h"
#include "InsidousBPFLibrary.h"
#include "EspectClient/Public/Net/SocketObject.h"
#include <Kismet/GameplayStatics.h>
#include "ProceduralMeshComponent.h"
#include "InsidousPlayerInterface.h"
#include "Materials/MaterialInstance.h"
#include "TimerManager.h"
#include <InsidousMediaCamera.h>
#include "UObject/SoftObjectPtr.h"

UInsidousGameInstance::UInsidousGameInstance()
{
}

void UInsidousGameInstance::Init()
{
	Super::Init();
	LoadDeviceSN();
}

FString UInsidousGameInstance::GetPlayerName_Implementation()
{
	return PlayerName;
}

int32 UInsidousGameInstance::GetInTeamIndex_Implementation()
{
	return InTeamIndex;
}

void UInsidousGameInstance::ResetCurrentSeqInfo()
{
	CurrentPathIndex = -1;
	TargetSeqSecond = 0;
}

// 打开 Map_ServerConnection 地图, 重新连接服务器
void UInsidousGameInstance::OpenMapServerConnection()
{	
	UGameplayStatics::OpenLevel(this, TEXT("/InsidousSDK/Code/Maps/Map_ServerConnection.Map_ServerConnection"));
}

void UInsidousGameInstance::LoadDeviceSN()
{
	FString path = UInsidousBPFLibrary::IsAndroid() ? "/sdcard/device_sn.txt" : FPaths::ProjectDir() + "/device_sn.txt";
	DeviceSN = UInsidousBPFLibrary::LoadFileToString(path);
	UE_LOG(LogTemp, Log, TEXT("device_sn: %s"), *DeviceSN);
}

void UInsidousGameInstance::ParseArena(FString data)
{
	MeshVertices.Empty();
	// parse json object
	FJsonLibraryObject ArenaJsonObject = FJsonLibraryObject::Parse(data);
	// 获取 JsonObject 的 arena 字段
	FJsonLibraryList shapes = ArenaJsonObject.GetList("shapes");
	for (int i = 0; i < shapes.Count(); i++)
	{
		FJsonLibraryObject ShapesJsonObject = shapes.GetObject(i);
		FJsonLibraryList points = ShapesJsonObject.GetList("points");
		TArray<FVector> vertices;
		for (int j = 0; j < points.Count(); j++)
		{
			FJsonLibraryObject point = points.GetObject(j);
			vertices.Add(FVector(point.GetFloat("x"), point.GetFloat("y"), 0.f));
		}
		FMeshVertices verts = {vertices};
		MeshVertices.Add(verts);
	}

	InsidousMediaCameras.Empty();
	FJsonLibraryList cameras = ArenaJsonObject.GetList("cameras");
	for (int i = 0; i < cameras.Count(); i++)
	{
		FJsonLibraryObject CameraJsonObject = cameras.GetObject(i);
		FInsidousMediaCam cam;
		cam.id = CameraJsonObject.GetInteger("id");
		cam.loc = FVector(CameraJsonObject.GetFloat("x"), CameraJsonObject.GetFloat("y") , CameraJsonObject.GetFloat("z"));
		cam.rot = FRotator(CameraJsonObject.GetFloat("ry"), CameraJsonObject.GetFloat("rz"), CameraJsonObject.GetFloat("rx"));
		InsidousMediaCameras.Add(cam);
	}

}

void UInsidousGameInstance::ParsePathInfo(FString data)
{
	DeployedPathLevelInfos.Empty();
	PathLevelInfos.Empty();
	FJsonLibraryObject JsonObject = FJsonLibraryObject::Parse(data);
	FJsonLibraryList levels = JsonObject.GetList("levels");
	for (int i = 0; i < levels.Count(); i++)
	{
		FJsonLibraryObject LevelJsonObject = levels.GetObject(i);
		bool in_stage = LevelJsonObject.GetBoolean("in_stage");
		if (!in_stage)
			continue;
		FPathLevelInfo info;
		info.path = LevelJsonObject.GetString("path");
		info.loc = FVector(LevelJsonObject.GetFloat("loc_x"), LevelJsonObject.GetFloat("loc_y"), LevelJsonObject.GetFloat("loc_z"));
		info.rot_z = LevelJsonObject.GetFloat("rot_z");
		info.start_loc = FVector(LevelJsonObject.GetFloat("start_x"), LevelJsonObject.GetFloat("start_y"), LevelJsonObject.GetFloat("start_z"));
		info.start_rot = LevelJsonObject.GetFloat("start_rot");
		PathLevelInfos.Add(info);
		DeployedPathLevelInfos.Add(info);
	}
}


void UInsidousGameInstance::OnFindPlayer(FString data)
{
	FJsonLibraryObject JsonObject = FJsonLibraryObject::Parse(data);
	AppName = JsonObject.GetString("app_name");
	contentVer = JsonObject.GetString("content_ver");
	contentName = JsonObject.GetString("content_name");
	PlayerName = JsonObject.GetString("name");
	InTeamIndex = FCString::Atoi(*JsonObject.GetString("in_team_index"));
	ParseArena(JsonObject.GetString("arena"));
	ParsePathInfo(JsonObject.GetString("path_str"));
	// 启动时的设置参数
	this->StartParams = JsonObject.GetString("start_params");
	OnFindPlayerBP();
}

void UInsidousGameInstance::OnStopPlay()
{
	ResetCurrentSeqInfo();
	OpenMapServerConnection();
}

void UInsidousGameInstance::OnStartPlay_Implementation()
{
	ResetCurrentSeqInfo();
	IFlowControlInterface::Execute_JumpToLevel(this, 0);
}

void UInsidousGameInstance::RegistSocketObject(USocketObject *obj)
{
	SocketObject = obj;
	if (SocketObject)
	{
		SocketObject->OnKickout.AddDynamic(this, &UInsidousGameInstance::OnKickout);
		SocketObject->OnGetServerMsg.AddDynamic(this, &UInsidousGameInstance::OnGetServerMsg);
		SocketObject->OnDisconnect.AddDynamic(this, &UInsidousGameInstance::OnDisconnect);
		SocketObject->OnReconnect.AddDynamic(this, &UInsidousGameInstance::OnReconnect);
	}
}

void UInsidousGameInstance::OnKickout(FJsonLibraryObject JsonObject)
{
	UE_LOG(LogTemp, Log, TEXT("socket kickout"));
}

void UInsidousGameInstance::OnGetServerMsg(FJsonLibraryObject JsonObject)
{
	IsConnected = true;
	FString server_msg_type = JsonObject.GetString("server_msg_type");
	FString server_msg_data = JsonObject.GetString("data");
	UE_LOG(LogTemp, Log, TEXT("socket get server msg: %s, %s"), *server_msg_type, *server_msg_data);
	if (server_msg_type == "on_start_play")
	{
		this->StartParams = server_msg_data;
		OnStartPlay();
	}
	if (server_msg_type == "on_find_player")
	{
		OnFindPlayer(server_msg_data);
	}
	if (server_msg_type == "on_stop_play")
	{
		OnStopPlay();
	}
	// 暂时不使用
	// 在不重启设备的情况下，重置所有信息，并重新连接服务和下载pak
	if (server_msg_type == "on_restart")
	{
		ResetCurrentSeqInfo();
		OpenMapServerConnection();
	}
}

void UInsidousGameInstance::OnDisconnect()
{
	UE_LOG(LogTemp, Log, TEXT("socket disconnect"));
	IsConnected = false;
	// 1秒以后 调用 socketObj 的 Reconnect 方法
	GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, [this]()
										   {
		if (SocketObject)
		{
			SocketObject->Reconnect();
		} }, 1.0f, false);
}
void UInsidousGameInstance::OnReconnect()
{
	UE_LOG(LogTemp, Log, TEXT("socket reconnect"));
	IsConnected = true;
	// 打开 Map_ServerConnection 地图, 重新连接服务器
	OpenMapServerConnection();
}


static FVector PositionTransform(const FVector& loc, const float& rot_z, USceneComponent* com)
{
	if (com == nullptr) return FVector();

	float x = loc.X * -1;
	float y = loc.Y * -1;
	float angle = rot_z * -1;

	// 定义新的相对位置和旋转
	FVector NewRelativeLocation = FVector(x, y, 0.0f); // 新的相对位置
	FRotator NewRelativeRotation = FRotator(0.0f, angle, 0.0f);    // 新的相对旋转

	float newX = x * FMath::Cos(UE_DOUBLE_PI / (180.0) * angle) - y * FMath::Sin(UE_DOUBLE_PI / (180.0) * angle) - x;
	float newY = x * FMath::Sin(UE_DOUBLE_PI / (180.0) * angle) + y * FMath::Cos(UE_DOUBLE_PI / (180.0) * angle) - y;

	com->SetRelativeLocationAndRotation(NewRelativeLocation + FVector(FVector(newX, newY, 0)), NewRelativeRotation);
	return FVector(x, y, angle);
}

// 主要是绘制虚拟墙，根据动线的位置调整玩家组件的位置
void UInsidousGameInstance::LevelStart_Implementation()
{
	// UE_LOG(LogTemp, Error, TEXT("LevelStart ............"));
	APawn *playerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (playerPawn && UKismetSystemLibrary::DoesImplementInterface(playerPawn, UInsidousPlayerInterface::StaticClass()))
	{
		USceneComponent *cameraRoot = IInsidousPlayerInterface::Execute_GetCameraRoot(playerPawn);
		if (!cameraRoot)
		{
			UE_LOG(LogTemp, Error, TEXT("cameraRoot is null"));
			return;
		}

		CreateArenaAndAttach(cameraRoot, playerPawn);
		CreateInsidousMediaCameraAndAttach(cameraRoot, playerPawn);
		// 获取当前 levelname
		FString levelName = UGameplayStatics::GetCurrentLevelName(this);
		AsyncLoadNextLevel();
		if (levelName != "Map_Waiting")
		{
			FGetCurrentPathResult result = IFlowControlInterface::Execute_GetCurrentPath(this);
			if (result.is_success)
			{
				PositionTransform(result.path.loc, result.path.rot_z, cameraRoot);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("get current path failed"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("playerPawn does not implement UInsidousPlayerInterface"));
	}
}

void UInsidousGameInstance::CreateArenaAndAttach(USceneComponent *attachTo, AActor *target)
{
	if (!attachTo || !target)
	{
		UE_LOG(LogTemp, Log, TEXT("attachTo or target is null"));
		return;
	}

	for (int i = 0; i < MeshVertices.Num(); i++)
	{
		UProceduralMeshComponent *mesh = NewObject<UProceduralMeshComponent>(target);
		if (mesh)
		{
			mesh->AttachToComponent(attachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			mesh->RegisterComponent();
			mesh->SetMaterial(0, ArenaMeshMaterial);
			TArray<FVector> vertices;
			TArray<int32> triangles;
			TArray<FVector> normals;
			TArray<FVector2D> uvs;
			TArray<FColor> colors;
			TArray<FProcMeshTangent> tangents;
			UInsidousBPFLibrary::CreateWallMesh(MeshVertices[i].Points, 250.0f, vertices, triangles, uvs);
			TArray<FVector2D> EmptyArray;
			mesh->CreateMeshSection(0, vertices, triangles, normals, uvs, EmptyArray, EmptyArray, EmptyArray, colors, tangents, true);
		}
	}
}

void UInsidousGameInstance::CreateInsidousMediaCameraAndAttach(USceneComponent* attachTo, AActor* target)
{
	if (!attachTo || !target)
	{
		UE_LOG(LogTemp, Log, TEXT("attachTo or target is null"));
		return;
	}

	for (auto i : InsidousMediaCameras)
	{
		UClass* BlueprintClass = StaticLoadClass(
			AActor::StaticClass(),
			nullptr,
			TEXT("Blueprint'/InsidousSDK/Code/Blueprints/BP_InsidousMediaCamera.BP_InsidousMediaCamera_C'")
		);

		if (BlueprintClass)
		{
			UWorld* World = GetWorld();
			AInsidousMediaCamera* cam = World->SpawnActorDeferred<AInsidousMediaCamera>(
				BlueprintClass,
				FTransform()
			);

			if (cam)
			{
				cam->Camera2D->SetRelativeLocationAndRotation(i.loc, i.rot);
				cam->id = i.id;
			}

			UGameplayStatics::FinishSpawningActor(cam, FTransform());


			//AInsidousMediaCamera* cam = GetWorld()->SpawnActorDeferred<AMotionControllerBase>(
			//	*ControllerClass, FTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			//leftHand->chirality = EControllerHand::Left;


			if (cam)
			{
				cam->AttachToComponent(attachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}

	}
}

void UInsidousGameInstance::AddOtherPlayerScale_Implementation(int32 userID, float scale)
{
	SeletalMeshScale.Add(userID, scale);
}

float UInsidousGameInstance::FindOtherPlayerScale_Implementation(int32 userID)
{
	if (SeletalMeshScale.Contains(userID))
	{
		return SeletalMeshScale[userID];
	}
	return 1.0f;
}

FOtherPlayerViewDistance UInsidousGameInstance::GetOtherPlayerViewDis_Implementation()
{
	FOtherPlayerViewDistance viewDistance;
	viewDistance.distance_cm = distance_cm;
	viewDistance.teammate_distance_cm = teammate_distance_cm;
	return viewDistance;
}

//void UInsidousGameInstance::SetCurrentPathIndex(int32 index)
//{
//	CurrentPathIndex = index;
//}

//void UInsidousGameInstance::SetTargetSeqSecond(int32 sec)
//{
//	TargetSeqSecond = sec;
//}

int32 UInsidousGameInstance::GetTargetSeqSecond_Implementation()
{
	return TargetSeqSecond;
}

//
//int32 UInsidousGameInstance::GetCurrentPathIndex()
//{
//	return CurrentPathIndex;
//}

FGetCurrentPathResult UInsidousGameInstance::GetCurrentPath_Implementation()
{
	FGetCurrentPathResult result;
	if (CurrentPathIndex >= 0 && CurrentPathIndex < PathLevelInfos.Num())
	{
		result.is_success = true;
		result.path = PathLevelInfos[CurrentPathIndex];
	}
	else
	{
		result.is_success = false;
	}
	return result;
}

void UInsidousGameInstance::AsyncLoadNextLevel()
{
	// 判断 CurrentPathIndex + 1 是否存在
	if (CurrentPathIndex >= 0 && CurrentPathIndex < PathLevelInfos.Num() - 1)
	{
		UInsidousBPFLibrary::AsyncLevelLoad(this, PathLevelInfos[CurrentPathIndex + 1].path, false);
	}
}

void UInsidousGameInstance::OnGetPlayerInfo(bool isLeader, int32 targetLevelIndex, bool needJumpToLevel, int32 targetSeqSec, bool needJumpToSeqSecond, int32 currentLevelIndex, int32 currentSeqSecond)
{
	// UE_LOG(LogTemp, Error, TEXT("OnGetPlayerInfo isLeader = %d tar_L = %d, need_L = %d, tar_S = %d , need_S = %d , cur_L = %d , cur_S = %d"), isLeader, targetLevelIndex, needJumpToLevel, targetSeqSec, needJumpToSeqSecond, currentLevelIndex, currentSeqSecond);
	if (needJumpToLevel)
	{
		if (needJumpToSeqSecond)
		{
			// 目标关卡打开以后，会根据此时间点开始播放
			TargetSeqSecond = targetSeqSec;
		}
		IFlowControlInterface::Execute_JumpToLevel(this,targetLevelIndex);
	}
	else
	{
		// isLeader 无条件执行序列Play()
		if (needJumpToSeqSecond || isLeader)
		{
			APawn *playerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
			if (playerPawn && UKismetSystemLibrary::DoesImplementInterface(playerPawn, UInsidousPlayerInterface::StaticClass()))
			{
				IInsidousPlayerInterface::Execute_SetMainSequenceTime(playerPawn, targetSeqSec, isLeader);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("playerPawn does not implement UInsidousPlayerInterface"));
			}
		}
	}
}
