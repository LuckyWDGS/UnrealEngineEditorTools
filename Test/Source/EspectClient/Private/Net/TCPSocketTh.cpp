// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/TCPSocketTh.h"
#include "Sockets.h"
#include "Net/SocketObject.h"
#include "Net/Bytebuf.h"
#include "RPCNodes.h"
#include <Net/LengthFieldBasedFrameDecoder.h>
#include "Async/TaskGraphInterfaces.h"
#include "Async/Async.h"
#include "Misc/Base64.h"
#include "SocketBPFLib.h"

#if WITH_EDITOR
#pragma optimize("",off)
#endif

FTCPSocketTh::FTCPSocketTh(USocketObject* _SocketObject) : bThreadRun(false), SocketObject(_SocketObject)
{
	Thread = FRunnableThread::Create(this, TEXT("TCP_RECEIVER"), 0, TPri_BelowNormal);
}

FTCPSocketTh::~FTCPSocketTh()
{
	delete Thread;
	Thread = nullptr;
}

bool FTCPSocketTh::Init()
{
	bThreadRun = true;
	return true;
}

FString GenKey()
{
	TArray<uint8> KeyBytes;
	FBase64::Decode("ajV5Vm53WDZ3RVR4djc4SjRrdlo5QUx4d1UwNnU1SlA5Z0pwT201b1luZ2l1RVRWSw==", KeyBytes);
	FString KeyString = FString(UTF8_TO_TCHAR(KeyBytes.GetData()));
	KeyString.RightChopInline(9);
	FString Left1 = KeyString.Left(17);
	KeyString.RightChopInline(25);
	FString Right = KeyString.Left(15);
	return Left1 + Right;
}

uint32 FTCPSocketTh::Run()
{
	if (!SocketObject) return 0;
	FLengthFieldBasedFrameDecoder* decoder = new FLengthFieldBasedFrameDecoder(16384, 0, 4, 4, 0);
	if (!decoder || !decoder->Init()) return 0;

	FByteBuf recvBuf(8 * 1024 * 1024);

	USocketObject* ThisSocketObject = SocketObject;

	uint32 Size = 0;
	int32 Readed = 0;
	recvBuf.Init(8 * 1024 * 1024);

	FString key = GenKey();
	
	Duration = 0;
	while (bThreadRun)
	{
		FDateTime StartTime = FDateTime::UtcNow();

		FPlatformProcess::Sleep(0.03f);
		if (!SocketObject->tcp_socket)
		{
			UE_LOG(LogTemp, Error, TEXT("SocketObject->tcp_socket is nullptr !!!"));
			return 0;
		}

		while (SocketObject && SocketObject->tcp_socket && SocketObject->tcp_socket->HasPendingData(Size)) // While there is something to read
		{
			SocketObject->tcp_socket->Recv(recvBuf.GetData() + recvBuf.GetWriterIndex(), recvBuf.Num(), Readed);
			recvBuf.SetWriterIndex(recvBuf.GetWriterIndex() + Readed);
		}

		if (recvBuf.readableBytes() > 0)
		{
			// UE_LOG(LogTemp, Log, TEXT("recv Bytes = %d"), recvBuf.readableBytes());
			TSharedRef<FByteBuf> decodedData = MakeShared<FByteBuf>(FMath::Min((uint32)recvBuf.readableBytes(), 1024u));
			while (decoder->decode(recvBuf, decodedData))
			{
				SocketObject->bIsConnected = true;

				uint16 Len = decodedData.Get().readInt();
				uint32 MagicNum = decodedData.Get().readShort();
				uint16 ReqID = decodedData.Get().readInt();
				uint32 MsgType = decodedData.Get().readShort();

				TArray<uint8> transdata;
				decodedData.Get().readBytes(Len - 4, &transdata);
				// 对数据进行解密
				FString dataString = USocketBPFLib::AESDecryptDytes(transdata, key);
				FJsonLibraryObject reqData = FJsonLibraryValue::Parse(dataString).GetObject();

				USocketObject* SocketObject_ = SocketObject;
				if (MsgType == 0) {} // UnKnow
				else if (MsgType == 1) {}// HeartBeat
				else if (MsgType == 2) // OnKickout
				{
					AsyncTask(ENamedThreads::GameThread, [SocketObject_, reqData]() {
						if (SocketObject_) {
							SocketObject_->OnKickout.Broadcast(reqData);
						}});
				}
				//else if (MsgType == 3) // OnDisconnected
				//{
				//	AsyncTask(ENamedThreads::GameThread, [SocketObject_]() {
				//		if (SocketObject_) {
				//			SocketObject_->OnDisconnected.Broadcast();
				//		}});
				//}
				else if (MsgType == 3) // OnGetServerMsg
				{
					AsyncTask(ENamedThreads::GameThread, [SocketObject_, reqData]() {
						if (SocketObject_) {
							SocketObject_->OnGetServerMsg.Broadcast(reqData);
						}});
				}
				else
				{
					URPCBase* caller = SocketObject_->GetCaller(ReqID);
					if (caller == nullptr || (caller != nullptr && !caller->IsValidLowLevel()) || !reqData.IsValid()) break;

					AsyncTask(ENamedThreads::GameThread, [caller, reqData]() {
						if (caller != nullptr && caller->IsValidLowLevel())
							caller->OnGetReturn(reqData);
						});
				}
			}
		}

		FDateTime EndTime = FDateTime::UtcNow();
		Duration += (EndTime - StartTime).GetTotalMilliseconds();
		if (Duration >= 10000.0f)
		{
			Duration = 0.0f;
			if (SocketObject->bIsConnected == false)
			{

				if (!SocketObject->SendHeartBeat())
				{
					this->bThreadRun = false;
					USocketObject* SocketObject_ = SocketObject;
					AsyncTask(ENamedThreads::GameThread, [SocketObject_]() {
						if (SocketObject_) {
							SocketObject_->OnDisconnect.Broadcast();
						}});

				}
			}
			SocketObject->bIsConnected = false;
		}
		
	}
	recvBuf.clear();
	return 0;
}

void FTCPSocketTh::Exit()
{
	bThreadRun = false;
}



#if WITH_EDITOR
#pragma optimize("",on)
#endif