#include "RPCNodes.h"
#include "Engine/World.h"
#include "TimerManager.h" // 添加这行


uint32 URPCBase::ReqID = 0;

void URPCBase::OnGetReturn(const FJsonLibraryObject& resp)
{
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
}

// 显式实例化模板函数
template void URPCBase::SetData<FGetServerInfoReq>(const FGetServerInfoReq& Struct);
template void URPCBase::SetData<FSetPlayerInfoReq>(const FSetPlayerInfoReq& Struct);
template void URPCBase::SetData<FSetPlayerPosReq>(const FSetPlayerPosReq& Struct);
template void URPCBase::SetData<FSetDeviceInfoReq>(const FSetDeviceInfoReq& Struct);
template void URPCBase::SetData<FSetDeviceContentInfoReq>(const FSetDeviceContentInfoReq& Struct);
template void URPCBase::SetData<FStopPlayReq>(const FStopPlayReq& Struct);
template void URPCBase::SetData<FTakeAPictureReq>(const FTakeAPictureReq& Struct);
template void URPCBase::SetData<FPlayerLogReq>(const FPlayerLogReq& Struct);


template<typename StructType>
void URPCBase::SetData(const StructType& Struct)
{
	// 1. 组装 data 部分为 byte 数组
	FJsonLibraryObject reqObj = FJsonLibraryObject(Struct);
	reqData = USocketBPFLib::ConvertJsonToBytes(reqObj);
}


void URPCBase::SetEmptyData()
{
	reqData.Init(0, 0);
}

void URPCBase::SetTimeout(float Timeout_)
{
	// 设置超时时间
	Timeout = Timeout_;
}



void URPCBase::Activate()
{
	this->id = URPCBase::GenerateReqID();
	RPCData Data;
	Data.req_id = this->id;
	Data.msg_type = (uint16)this->MsgType;
	Data.len = this->reqData.Num();
	Data.data = this->reqData.GetData();
	// 3. 发送
	SocketObject = USocketBPFLib::GetSocketClientManager()->GetSocketObject();
	if (SocketObject && SocketObject->IsValidLowLevel())
	{
		SocketObject->Send(this, &Data);
		GetWorld()->GetTimerManager().SetTimer(TimeoutTimerHandle, this, &URPCBase::OnTimeout, Timeout, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SocketObject is nullptr, cannot send message !"));
	}
}

void URPCBase::OnTimeout()
{
	UE_LOG(LogTemp, Error, TEXT("%s timeout!"), *this->GetFName().ToString());
}

// ...
template <typename StructType>
void URPCBase::SendData(const StructType& Struct)
{
	// 1. 组装 data 部分为 byte 数组
	FJsonLibraryObject reqObj = FJsonLibraryObject(Struct);
	reqData = USocketBPFLib::ConvertJsonToBytes(reqObj);
	// 2. 组装一个 RPCData 数据

	this->id = URPCBase::GenerateReqID();
	RPCData Data;
	Data.req_id = this->id;
	Data.msg_type = MsgType;
	Data.len = reqData.Num();
	Data.data = reqData.GetData();
	// 3. 发送
	this->SocketObject->Send(this, &Data);
}