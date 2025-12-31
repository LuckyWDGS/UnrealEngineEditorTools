// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Bytebuf.h"
#include "JsonLibrary.h"
#include "SocketObject.generated.h"

class FSocket;
class FTCPSocketTh;
class UHandlerBase;
class URPCBase;
class FInternetAddr;

struct RPCData{

	uint32 len;
	uint32 req_id;
	uint16 msg_type;
	uint8* data;

	uint32 GetTotalLen();
	void ToBytes(FByteBuf& buf);

};

UCLASS(BlueprintType)
class ESPECTCLIENT_API USocketObject : public UObject
{
	GENERATED_BODY()

	~USocketObject();

	// 接收来自服务器的消息
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKickout, FJsonLibraryObject, msg);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisconnected);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReconnected);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetServerMsg, FJsonLibraryObject, msg);

public:
	FSocket *tcp_socket;
	TSharedPtr<FInternetAddr> tcp_address;
	bool bIsConnected;
	UFUNCTION(BlueprintCallable, Category = "EspectClient")
	void Reconnect();

	bool SendHeartBeat();

	bool InitSocket(FString serverAddress_, int32 tcp_server_port_);

	UPROPERTY(BlueprintAssignable)
		FOnKickout OnKickout;

	UPROPERTY(BlueprintAssignable)
		FOnDisconnected OnDisconnect;

	UPROPERTY(BlueprintAssignable)
		FOnReconnected OnReconnect;

	UPROPERTY(BlueprintAssignable)
		FOnGetServerMsg OnGetServerMsg;


	UFUNCTION(BlueprintCallable, Category = "EspectClient")
	void CloseClient();

	bool Send(URPCBase* Caller, RPCData* Data);

#pragma region callers
		URPCBase* GetCaller(uint32 CallerID);
#pragma endregion


private:

	FString serverAddress;
	int32 tcp_server_port;

	TMap<uint32, URPCBase*> RpcCallers;
	FTCPSocketTh *Runnable;
};