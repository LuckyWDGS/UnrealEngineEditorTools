// Fill out your copyright notice in the Description page of Project Settings.

#include <Net/SocketObject.h>
#include "Sockets.h"
// #include "Networking/Public/Networking.h"
// #include <Networking/Public/Common/UdpSocketReceiver.h>
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Net/TCPSocketTh.h"
#include "RPCNodes.h"
#include <Net/Bytebuf.h>


#if WITH_EDITOR
#pragma optimize("", off)
#endif

USocketObject::~USocketObject()
{
	UE_LOG(LogTemp, Log, TEXT("~USocketObject() ~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));

	if (tcp_socket != nullptr)
	{
		tcp_socket->Close();
		delete tcp_socket;
		tcp_socket = nullptr;
	}

	if (Runnable)
	{
		Runnable->Exit();
		delete Runnable;
		Runnable = nullptr;
	}
}


bool USocketObject::InitSocket(FString serverAddress_, int32 tcp_server_port_)
{
	serverAddress = serverAddress_;
	tcp_server_port = tcp_server_port_;
	
	if (FPlatformProcess::SupportsMultithreading())
	{

#if PLATFORM_ANDROID
		ISocketSubsystem* sSS = ISocketSubsystem::Get(FName(TEXT("ANDROID")));
#else
		ISocketSubsystem* sSS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
#endif

		tcp_socket = sSS->CreateSocket(NAME_Stream, TEXT("TCP_SOCKET"), false);
		tcp_address = sSS->CreateInternetAddr();
		FIPv4Address serverIP;
		FIPv4Address::Parse(serverAddress, serverIP);
		tcp_address->SetIp(serverIP.Value);
		tcp_address->SetPort(tcp_server_port);

		if (tcp_socket == nullptr || tcp_socket->Connect(*tcp_address) == false) {
			const TCHAR* socketErr = sSS->GetSocketError(SE_GET_LAST_ERROR_CODE);
			FString info = FString(socketErr) + "|" + serverAddress + ":" + FString::FromInt(tcp_server_port);
			UE_LOG(LogTemp, Warning, TEXT("Connection failed(2): %s"), *info);
		}
		else
		{
			bIsConnected = true;
			Runnable = new FTCPSocketTh(this);

		}
	}
	else
	{
		return false;
	}
	return bIsConnected;
}


void USocketObject::CloseClient()
{
	if (tcp_socket)
	{
		tcp_socket->Close();
	}

	if (Runnable)
	{
		Runnable->Exit();
	}
}

bool USocketObject::Send(URPCBase* Caller, RPCData* Data)
{
	
	if (Data == nullptr) { return false; }
	
	if( tcp_socket )
	{
		if (Caller != nullptr)
			RpcCallers.Add(Caller->GetID(), Caller);

		uint32 totalLen = Data->GetTotalLen();
		FByteBuf sendBuffer(totalLen, totalLen);
		Data->ToBytes(sendBuffer);
		int32 BytesSent;

		if (tcp_socket->Send(sendBuffer.GetBuf()->GetData(), totalLen, BytesSent))
		{
			// UE_LOG(LogTemp, Log, TEXT("BytesSent = %d"), BytesSent);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Send failed !!!"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("tcp_socket = nullptr"));
		return false;
	}
}

URPCBase* USocketObject::GetCaller(uint32 callerID)
{
	auto res = this->RpcCallers.Find(callerID);
	if (res != nullptr)
	{
		return *res;
	}else
	{
		return nullptr;
	}
}

void USocketObject::Reconnect()
{
	tcp_socket->Close();
	bool res = InitSocket(serverAddress, tcp_server_port);
	if (res)
	{
		Runnable->SetThreadRun(true);
		OnReconnect.Broadcast();
	}
	else
	{
		OnDisconnect.Broadcast();
	}
}


bool USocketObject::SendHeartBeat()
{
	TArray<uint8> reqData;
	reqData.Init(0, 0);
	RPCData Data;
	Data.req_id = 0;
	Data.msg_type = 1;
	Data.len = reqData.Num();
	Data.data = reqData.GetData();

	UE_LOG(LogTemp, Log, TEXT("SendHeartBeat !!!"));
	return Send(nullptr, &Data);
}


uint32 RPCData::GetTotalLen()
{
	return 12 + len;
}

void RPCData::ToBytes(FByteBuf& buf)
{
	uint16 magic_num = 0x0DEC;
	buf
		.writeInt(buf.Num() - 4)
		.writeShort(magic_num)
		.writeInt(req_id)
		.writeShort(msg_type)
		.writeBytes(this->data, len);
}


#if WITH_EDITOR
#pragma optimize("",on)
#endif