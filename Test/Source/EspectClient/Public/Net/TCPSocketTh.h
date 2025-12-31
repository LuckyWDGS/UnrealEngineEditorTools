// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"

class USocketObject;
class ESPECTCLIENT_API FTCPSocketTh : public FRunnable
{
	FRunnableThread *Thread;
	bool bThreadRun;
	USocketObject *SocketObject;

	// 单位毫秒
	float Duration = 0.f;

public:
	FTCPSocketTh(USocketObject *_SocketObject);
	~FTCPSocketTh();

	void SetThreadRun(bool Run_) { bThreadRun = Run_; }
	virtual bool Init();
	virtual uint32 Run();
	virtual void Exit();
};
