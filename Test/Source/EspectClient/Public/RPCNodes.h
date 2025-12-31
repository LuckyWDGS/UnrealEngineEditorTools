#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include <Net/SocketObject.h>
#include "UObject/StructOnScope.h"
#include "Engine/TimerHandle.h"
#include "JsonLibrary.h"
#include "SocketBPFLib.h"
#include "RPCNodes.generated.h"



// 消息类型
enum RPCMessageType : uint16
{
	
	Unknown, // 0
	HeartBeat, // 1
	OnKickout, // 2
	OnGetServerMsg, // 3
	SetConnectionType, // 4

	// 玩家相关 用10000开头的
	// 控制端相关 用20000开头的
	SetPlayerInfo = 10001, // 获取绑定的玩家的信息
	GetServerInfo, // 获取服务器部署的内容和动线信息
	SetPlayerPos, // 发送玩家的坐标信息
	GetServerTimeUtcGmt, // 获取系统的时间，主要是用于更新OBS
	SetDeviceInfo, // 同步设备电量等信息
	SetDeviceContentInfo, // 同步设备内容播放状态
	StopPlay, // 服务器事件，请求结束播放
	TheEnd, // 内容播放完
	TakeAPicture, // 游后分享拍照功能
	PlayerLog, // 客户端网服务器发送日志信息
};

// 客户端类型
UENUM(BlueprintType)
enum class FConnectionType : uint8
{
	UEServer,
	Player,
};

// 世界的信息
USTRUCT(BlueprintType)
struct FServerRoomInfo
{
	GENERATED_BODY()

	// room_port
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 p;

	// app_id
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 ai;

	// app_version
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 av;

	// app_name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString an;

	// app_file_name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString afn;

	// app_name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString adn;

	// app_cover_path
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString acp;

	// creater_name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString ci;

	// creater_name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString cn;

	// member_num
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 m;

	// observer_num
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 o;

	// is_start
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool s;
};


USTRUCT(BlueprintType)
struct FPlayerDataLite
{
	GENERATED_BODY()
	// 玩家id uid
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString a;

	// 当前房间的编号 current_room_port
	// -1 大厅
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rp;

	// 房间名称 room_name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString rn;

	// 训练是否已经开始 is_start
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool s;
};


// 返回结果代理（只有一个整数的返回参数） -- 公用的
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOneIntResult, int32, Result);

// 返回结果代理（只有一个布尔值的返回参数） -- 公用的
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOneBoolResult, bool, Result);

// 返回结果代理（只有一个字符串的返回参数） -- 公用的
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOneStringResult, FString, Result);


#pragma region URPCBase
/**
 *	RPC 函数的基类
 */
UCLASS()
class ESPECTCLIENT_API URPCBase  : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	FTimerHandle TimeoutTimerHandle;

public:
	// 一次RPC调用的ID
	static uint32 ReqID;

	// 生成RPC调用ID，每次加1
	static uint32 GenerateReqID() { return ++ReqID; }

	//~URPCBase(){ 
	//if(GetWorld()) GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	//		
	//UE_LOG(LogTemp, Log, TEXT("~URPCBase ~~~~~~~"));
	//}

	// 当获取到返回值的时候
	virtual void OnGetReturn(const FJsonLibraryObject& resp);

	uint32 GetID() { return this->id; }

	template<typename StructType>
	void SetData(const StructType& Struct);

	void SetEmptyData();

	void SetTimeout(float Timeout_);


protected:
	uint32 id = 0;
	RPCMessageType MsgType;
	TArray<uint8> reqData;
	float Timeout = 5;
	USocketObject* SocketObject;

	virtual void Activate() override;

	virtual void OnTimeout();

	// ...
	template <typename StructType>
	void SendData(const StructType& Struct);
};
#pragma endregion


// 返回服务器的其他信息，比如文件服务器的IP, 设备的SN 对应的唯一ID
#pragma region GetServerInfo


// 请求的参数
USTRUCT(BlueprintType)
struct FGetServerInfoReq
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString sn;

	// 
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 conn_type = 0;
};

USTRUCT(BlueprintType)
struct FContentInfo
{
	GENERATED_USTRUCT_BODY()
	// 应用名称，如果是安卓平台，那么就是 android_package_name: com.nasu.xxx
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString app_name;
	// DLC 目录名称，就是UE content/crates/xxx 路径中对应的 xxx
	// 大部分情况下都应该使用 app_name
	// 只有在引擎中寻找和加载对应的资产时，需要用到该 content_name
	// 为什么不和 app_name 保持一致呢，因为如果上传应用时发现重名
	// 则可以修改 app_name，但是修改 UE 文件名的代价太高，需要重新修复索引，重新烘焙打包等
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString content_name;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString content_ver;
};

// 服务器返回的参数
USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 device_id;

	// 127.0.0.1:9000
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString file_server_host;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	TArray<FContentInfo> contents;

	//UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	//FString arena;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString server_time;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 distance_cm;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 teammate_distance_cm;
};

UCLASS()
class ESPECTCLIENT_API URPC_GetServerInfo : public URPCBase
{
	GENERATED_BODY()
public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetServerInfoResult, const FServerInfo&, Result);

	UPROPERTY(BlueprintAssignable)
	FGetServerInfoResult OnCompleted;

	// No return data in 5 seconds.
	UPROPERTY(BlueprintAssignable)
	FGetServerInfoResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_GetServerInfo* RPC_GetServerInfo(UObject* WorldContextObject, const FGetServerInfoReq& param)
	{
		URPC_GetServerInfo* Node = NewObject<URPC_GetServerInfo>(WorldContextObject);
		Node->MsgType = RPCMessageType::GetServerInfo;

		Node->SetData(param);
		return Node;
	}

	void OnGetReturn(const FJsonLibraryObject& resp) override
	{
		Super::OnGetReturn(resp);

		auto data = resp.ToStruct(FServerInfo::StaticStruct());
		FServerInfo* info = (FServerInfo*)data.Get()->GetStructMemory();

		OnCompleted.Broadcast(*info);
	}

	void OnTimeout() override
	{
		Super::OnTimeout();
		OnFailed.Broadcast(FServerInfo{});
	}
};

#pragma endregion

#pragma region SetPlayerInfo

// 请求的参数
USTRUCT(BlueprintType)
struct FSetPlayerInfoReq
{
	GENERATED_USTRUCT_BODY()
	// 关卡序列是否已经播放结束了， finished
	// 播放结束后需要跳转到下一个关卡
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool f;

	// 关卡序号， 等待关卡是 -1
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 l;

	// 体验区
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	uint8 t;

	// 序列帧的秒数
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 m;

	// in vo
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool v;
};

// 返回的时序信息，如果小组时序一致，则不会返回
USTRUCT(BlueprintType)
struct FSetPlayerInfoResp
{
	GENERATED_USTRUCT_BODY()

	// 是否是组长
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool ld;

	// 关卡序号， 等待关卡是 -1
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 l;

	// 是否需要跳转关卡
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool bl;

	// 序列帧的秒数
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 m;

	// 是否需要跳转sequence
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool bm;

};


UCLASS()
class ESPECTCLIENT_API URPC_SetPlayerInfo : public URPCBase
{
	GENERATED_BODY()
public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetPlayerInfoResult, const FSetPlayerInfoResp&, Result);

	UPROPERTY(BlueprintAssignable)
	FSetPlayerInfoResult OnCompleted;

	// No return data in 5 seconds.
	UPROPERTY(BlueprintAssignable)
	FSetPlayerInfoResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_SetPlayerInfo* RPC_SetPlayerInfo(UObject* WorldContextObject, const FSetPlayerInfoReq& param)
	{
		URPC_SetPlayerInfo* Node = NewObject<URPC_SetPlayerInfo>(WorldContextObject);
		Node->MsgType = RPCMessageType::SetPlayerInfo;

		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(5);

		return Node;
	}

	void OnGetReturn(const FJsonLibraryObject& resp) override
	{
		Super::OnGetReturn(resp);

		auto data = resp.ToStruct(FSetPlayerInfoResp::StaticStruct());
		FSetPlayerInfoResp* info = (FSetPlayerInfoResp*)data.Get()->GetStructMemory();

		OnCompleted.Broadcast(*info);
	}

	void OnTimeout() override
	{
		Super::OnTimeout();
		OnFailed.Broadcast(FSetPlayerInfoResp{});
	}

};

#pragma endregion



#pragma region SetPlayerPos

// 发送玩家的坐标 和 获取其他玩家的信息
USTRUCT(BlueprintType)
struct FSetPlayerPosReq
{
	GENERATED_USTRUCT_BODY()
	// 每次必须携带的设备ID!!!
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 i;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 x;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 y;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 z;
	// Z 轴的旋转
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 llx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lly;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 llz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lrx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lry;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lrz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rlx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rly;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rlz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rrx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rry;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rrz;
};


USTRUCT(BlueprintType)
struct FSetPlayerPosResp
{
	GENERATED_USTRUCT_BODY()
	// USER id
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 i;

	// name
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString n;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 x;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 y;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 z;

	// Z 轴的旋转
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 llx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lly;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 llz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lrx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lry;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 lrz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rlx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rly;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rlz;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rrx;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rry;
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 rrz;

	// 性别
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 g;
};


UCLASS()
class ESPECTCLIENT_API URPC_SetPlayerPos : public URPCBase
{
	GENERATED_BODY()
public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetPlayerPosResult, const TArray<FSetPlayerPosResp>&, Result);

	UPROPERTY(BlueprintAssignable)
	FSetPlayerPosResult OnCompleted;

	// No return data in 5 seconds.
	UPROPERTY(BlueprintAssignable)
	FSetPlayerPosResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_SetPlayerPos* RPC_SetPlayerPos(UObject* WorldContextObject, const FSetPlayerPosReq& param, const float Timeout = 0)
	{
		URPC_SetPlayerPos* Node = NewObject<URPC_SetPlayerPos>(WorldContextObject);
		Node->MsgType = RPCMessageType::SetPlayerPos;
		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(Timeout);

		return Node;
	}

	void OnGetReturn(const FJsonLibraryObject& resp) override
	{
		Super::OnGetReturn(resp);
		// GetWorldsResp resp;
		// resp.ParseFromArray(resp.GetData(), resp.Num());
		TArray<FSetPlayerPosResp> PlayersInfo;
		auto DataList = resp.GetArray("data");
		for (auto i : DataList)
		{
			auto aa = i.GetObject().ToStruct(FSetPlayerPosResp::StaticStruct());
			FSetPlayerPosResp* bb = (FSetPlayerPosResp*)aa.Get()->GetStructMemory();
			PlayersInfo.Add(*bb);
		}

		OnCompleted.Broadcast(PlayersInfo);
	}

	// 不考虑超时，不考虑失败的情况
	void OnTimeout() override {}

};
#pragma endregion


#pragma region SetDeviceInfo

// 请求的参数
USTRUCT(BlueprintType)
struct FSetDeviceInfoReq
{
	GENERATED_USTRUCT_BODY()

	// 电量 battary
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	uint8 b;

	// 追踪状态，trackingState
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool s;

	// 正在体验 is_playing
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	bool p;
};


UCLASS()
class ESPECTCLIENT_API URPC_SetDeviceInfo : public URPCBase
{
	GENERATED_BODY()
public:

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetDeviceInfoResult, const FSetDeviceInfoResp&, Result);

	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnCompleted;

	//// No return data in 5 seconds.
	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_SetDeviceInfo* RPC_SetDeviceInfo(UObject* WorldContextObject, const FSetDeviceInfoReq& param)
	{
		URPC_SetDeviceInfo* Node = NewObject<URPC_SetDeviceInfo>(WorldContextObject);
		Node->MsgType = RPCMessageType::SetDeviceInfo;

		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(0);

		return Node;
	}

	//void OnGetReturn(const FJsonLibraryObject& resp) override
	//{
	//	Super::OnGetReturn(resp);
	//	OnCompleted.Broadcast(*info);
	//}

	//// 不考虑超时，不考虑失败的情况
	//void OnTimeout() override {}

};

#pragma endregion

#pragma region GetServerTime

USTRUCT(BlueprintType)
struct FGetServerTimeResp
{
	GENERATED_USTRUCT_BODY()
	// USER id
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString time_utc_gmt;
};

UCLASS()
class ESPECTCLIENT_API URPC_GetServerTimeUTC_GMT : public URPCBase
{
	GENERATED_BODY()
public:


	UPROPERTY(BlueprintAssignable)
	FOneStringResult OnCompleted;

	// No return data in 5 seconds.
	UPROPERTY(BlueprintAssignable)
	FOneStringResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_GetServerTimeUTC_GMT* RPC_GetServerTimeUTC_GMT(UObject* WorldContextObject)
	{
		URPC_GetServerTimeUTC_GMT* Node = NewObject<URPC_GetServerTimeUTC_GMT>(WorldContextObject);
		Node->MsgType = RPCMessageType::GetServerTimeUtcGmt;
		return Node;
	}

	void OnGetReturn(const FJsonLibraryObject& resp) override
	{
		Super::OnGetReturn(resp);

		auto data = resp.ToStruct(FGetServerTimeResp::StaticStruct());
		FGetServerTimeResp* info = (FGetServerTimeResp*)data.Get()->GetStructMemory();

		OnCompleted.Broadcast(*info->time_utc_gmt);
	}

	// 不考虑超时，不考虑失败的情况
	void OnTimeout() override {}

};

#pragma endregion


#pragma region SetDeviceContentInfo

// 请求的参数
USTRUCT(BlueprintType)
struct FSetDeviceContentInfoReq
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString content_name;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString content_display_name;

	// 追踪状态，trackingState
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString content_version;
};


UCLASS()
class ESPECTCLIENT_API URPC_SetDeviceContentInfo : public URPCBase
{
	GENERATED_BODY()
public:

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetDeviceInfoResult, const FSetDeviceInfoResp&, Result);

	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnCompleted;

	//// No return data in 5 seconds.
	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_SetDeviceContentInfo* RPC_SetDeviceContentInfo(UObject* WorldContextObject, const FSetDeviceContentInfoReq& param)
	{
		URPC_SetDeviceContentInfo* Node = NewObject<URPC_SetDeviceContentInfo>(WorldContextObject);
		Node->MsgType = RPCMessageType::SetDeviceContentInfo;

		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(0);

		return Node;
	}

	//void OnGetReturn(const FJsonLibraryObject& resp) override
	//{
	//	Super::OnGetReturn(resp);
	//	OnCompleted.Broadcast(*info);
	//}

	//// 不考虑超时，不考虑失败的情况
	//void OnTimeout() override {}

};
#pragma endregion



#pragma region StopPlay

// 请求的参数
USTRUCT(BlueprintType)
struct FStopPlayReq
{
	GENERATED_USTRUCT_BODY()

	// 结束体验的原因
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString reason;
};

UCLASS()
class ESPECTCLIENT_API URPC_StopPlay : public URPCBase
{
	GENERATED_BODY()
public:

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetDeviceInfoResult, const FSetDeviceInfoResp&, Result);

	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnCompleted;

	//// No return data in 5 seconds.
	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_StopPlay* RPC_StopPlay(UObject* WorldContextObject, const FStopPlayReq& param)
	{
		URPC_StopPlay* Node = NewObject<URPC_StopPlay>(WorldContextObject);
		Node->MsgType = RPCMessageType::StopPlay;

		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(0);

		return Node;
	}

	//void OnGetReturn(const FJsonLibraryObject& resp) override
	//{
	//	Super::OnGetReturn(resp);
	//	OnCompleted.Broadcast(*info);
	//}

	//// 不考虑超时，不考虑失败的情况
	//void OnTimeout() override {}

};
#pragma endregion

#pragma region TheEnd 
// 请求的参数
UCLASS()
class ESPECTCLIENT_API URPC_TheEnd : public URPCBase
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_TheEnd* RPC_TheEnd(UObject* WorldContextObject)
	{
		URPC_TheEnd* Node = NewObject<URPC_TheEnd>(WorldContextObject);
		Node->MsgType = RPCMessageType::TheEnd;
		// 自定义超时时长
		Node->SetTimeout(0);

		return Node;
	}
};
#pragma endregion



#pragma region TakeAPicture

// 记得在 成cpp 中
// 显式实例化模板函数，否则打包会失败
// template void URPCBase::SetData<FTakeAPictureReq>(const FTakeAPictureReq& Struct);

// 请求的参数
// 虚拟拍照命名 uuid_相机ID_照片序号_V
// 拍照命名 小组id_用户id_用户名_相机ID_照片序号_R
// 合成照片命名 小组id_用户id_用户名_相机ID_照片序号_R
USTRUCT(BlueprintType)
struct FTakeAPictureReq
{
	GENERATED_USTRUCT_BODY()

	// 相机序号
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	int32 camera_id;

	// picture index
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString picture_index;

	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString virtual_picture_path;
};

UCLASS()
class ESPECTCLIENT_API URPC_TakeAPicture : public URPCBase
{
	GENERATED_BODY()
public:

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetDeviceInfoResult, const FSetDeviceInfoResp&, Result);

	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnCompleted;

	//// No return data in 5 seconds.
	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_TakeAPicture* RPC_TakeAPicture(UObject* WorldContextObject, const FTakeAPictureReq& param)
	{
		URPC_TakeAPicture* Node = NewObject<URPC_TakeAPicture>(WorldContextObject);
		Node->MsgType = RPCMessageType::TakeAPicture;

		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(0);

		return Node;
	}

	//void OnGetReturn(const FJsonLibraryObject& resp) override
	//{
	//	Super::OnGetReturn(resp);
	//	OnCompleted.Broadcast(*info);
	//}

	//// 不考虑超时，不考虑失败的情况
	//void OnTimeout() override {}

};
#pragma endregion



#pragma region PlayerLog


UENUM(BlueprintType)
enum class EPlayerLogType : uint8 {
	/// 玩家信息日志
	PlayerInfoLog = 0,
	/// 玩家警告日志
	PlayerWarningLog = 1,
	/// 玩家错误日志
	PlayerErrorLog = 2,
};

// 记得在 成cpp 中
// 显式实例化模板函数，否则打包会失败
// template void URPCBase::SetData<FTakeAPictureReq>(const FTakeAPictureReq& Struct);

// 请求的参数
// 虚拟拍照命名 uuid_相机ID_照片序号_V
// 拍照命名 小组id_用户id_用户名_相机ID_照片序号_R
// 合成照片命名 小组id_用户id_用户名_相机ID_照片序号_R
USTRUCT(BlueprintType)
struct FPlayerLogReq
{
	GENERATED_USTRUCT_BODY()

	// 相机序号
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	EPlayerLogType log_type;

	// picture index
	UPROPERTY(BlueprintReadWrite, Category = "EspectRPC")
	FString log;
};

UCLASS()
class ESPECTCLIENT_API URPC_PlayerLog : public URPCBase
{
	GENERATED_BODY()
public:

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetDeviceInfoResult, const FSetDeviceInfoResp&, Result);

	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnCompleted;

	//// No return data in 5 seconds.
	//UPROPERTY(BlueprintAssignable)
	//FOneIntResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "EspectRPC"))
	static URPC_PlayerLog* RPC_PlayerLog(UObject* WorldContextObject, const FPlayerLogReq& param)
	{
		URPC_PlayerLog* Node = NewObject<URPC_PlayerLog>(WorldContextObject);
		Node->MsgType = RPCMessageType::PlayerLog;

		Node->SetData(param);
		// 自定义超时时长
		Node->SetTimeout(0);

		return Node;
	}

	//void OnGetReturn(const FJsonLibraryObject& resp) override
	//{
	//	Super::OnGetReturn(resp);
	//	OnCompleted.Broadcast(*info);
	//}

	//// 不考虑超时，不考虑失败的情况
	//void OnTimeout() override {}

};
#pragma endregion