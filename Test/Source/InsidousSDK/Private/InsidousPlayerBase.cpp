// Fill out your copyright notice in the Description page of Project Settings.

#include "InsidousPlayerBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FlowControlInterface.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AInsidousPlayerBase::AInsidousPlayerBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInsidousPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && UKismetSystemLibrary::DoesImplementInterface(GameInstance, UFlowControlInterface::StaticClass()))
	{
		IFlowControlInterface::Execute_LevelStart(GameInstance);
		// 每个关卡开始时，记录一下用户的结束时间
		// 也可以由内容在任何时候再次调用，以记录用户体验结束的时间
		IFlowControlInterface::Execute_TheEnd(GameInstance);
	}

	// 1秒以后 调用 socketObj 的 Reconnect 方法
	//GetWorld()->GetTimerManager().SetTimer(GetOtherActorsTimeHandle, [this]()
	//	{
	//		// 需要延时获取时序信息
	//		this->TryGetMainSequencerPlayer();

	//		if (SequencePlayer)
	//		{
	//			SequencePlayer->OnFinished.AddDynamic(this, &AInsidousPlayerBase::OnMainSequencerFinished);

	//			// 第一次获取到MainSequencerPlayer 时，调用LevelStart 接口，并设置目标时间
	//			int32 targetSeqSecond = 0;
	//			UGameInstance* GameInstance = GetGameInstance();
	//			if (GameInstance && UKismetSystemLibrary::DoesImplementInterface(GameInstance, UFlowControlInterface::StaticClass()))
	//			{
	//				IFlowControlInterface::Execute_LevelStart(GameInstance);
	//				targetSeqSecond = IFlowControlInterface::Execute_GetTargetSeqSecond(GameInstance);

	//				// 每个关卡开始时，记录一下用户的结束时间
	//				// 也可以由内容在任何时候再次调用，以记录用户体验结束的时间
	//				IFlowControlInterface::Execute_TheEnd(GameInstance);
	//			}
	//			SetMainSequenceTime_Implementation(targetSeqSecond);
	//		}

	//	}, 0.8f, false);
}

void AInsidousPlayerBase::TryGetMainSequencerPlayer()
{
	TArray<AActor *> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALevelSequenceActor::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		ALevelSequenceActor *seqActor = Cast<ALevelSequenceActor>(Actors[0]);
		if( seqActor != nullptr)
		{
			SequencePlayer = seqActor->GetSequencePlayer();
			SequencePlayer->OnFinished.AddDynamic(this, &AInsidousPlayerBase::OnMainSequencerFinished);
		}

	}
}

void AInsidousPlayerBase::InsidousPause(bool Pause)
{
	ManualPaused = Pause;
	if(Pause)
	{
		if (SequencePlayer) SequencePlayer->Pause();
	}
	else
	{
		if (SequencePlayer) SequencePlayer->Play();
	}
}

// Called every frame
void AInsidousPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (LevelManagerActor == nullptr)
	{
		LevelManagerActor = TryGetLevelManagerActor();
	}


	if (Vehicle == nullptr)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		if (LevelManagerActor)
		{
			LevelManagerActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
		AttachedVehicle = nullptr;
	}
	else if (Vehicle != AttachedVehicle)
	{
		// 通过 Actor 中的 组件名称 StartPoint 获取到 组件
		// 可以遍历所有组件，获取到 StartPoint 组件	
		USceneComponent *StartPoint = nullptr;
		for (auto Component : Vehicle->GetComponents())
		{
			if (Component->GetName() == "StartPoint")
			{
				StartPoint = Cast<USceneComponent>(Component);
			}
		}
		if (StartPoint)
		{
			AttachToComponent(StartPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			UE_LOG(LogTemp, Log, TEXT("Attached to Vehicle StartPoint"));

			if (LevelManagerActor)
			{
				LevelManagerActor->AttachToComponent(StartPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				AttachedVehicle = Vehicle;
				
				// 载具中如果有 OnLogging 的函数，那么在使用该载具时，该函数会被调用
				// 通过反射系统查找函数对象
				UFunction* Function = Vehicle->FindFunction(TEXT("OnLogging"));

				FPlayerParam Params;
				Params.player = this;
				// 调用函数
				if(Function) { Vehicle->ProcessEvent(Function, &Params); }
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("LevelManager is nullptr"));
			}
		}
		else
		{
			DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			if (LevelManagerActor)
			{
				LevelManagerActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			}
			AttachedVehicle = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Vehicle StartPoint is nullptr"));
		}
	}
}

// 主序列播放完成时调用
void AInsidousPlayerBase::OnMainSequencerFinished()
{
	LevelFinished = true;
}

void AInsidousPlayerBase::SetMainSequenceTime_Implementation(int32 sec, bool isTeamLeader)
{
	// 如果手动暂停了
	if (ManualPaused)
	{
		return;
	}

	if (SequencePlayer)
	{
		auto seqTime = SequencePlayer->GetCurrentTime();
		int32 currentTime = seqTime.Time.FrameNumber.Value / seqTime.Rate.Numerator;

		currentTime = currentTime < 0 ? 0 : currentTime;
		// 获取当前 levelname
		FString levelName = UGameplayStatics::GetCurrentLevelName(this);
		// 如果是组长，则无条件 Play(), 玩家 BeginPlay 中第一次调用时，需设置 isTeamLeader 为 true

		if (isTeamLeader)
		{
			if(!SequencePlayer->IsPlaying())
			{
				FMovieSceneSequencePlaybackParams Params = FMovieSceneSequencePlaybackParams((float)sec, EUpdatePositionMethod::Jump);
				SequencePlayer->SetPlaybackPosition(Params);
				SequencePlayer->Play();
				UE_LOG(LogTemp, Warning, TEXT("SetMainSequenceTime Leader Play() currentTime = %d target_sec = %d level = %s, leader = %d"), currentTime, sec, *levelName, isTeamLeader);
				return;
			}
		}
		else
		{
			if (currentTime < sec)
			{
				UE_LOG(LogTemp, Warning, TEXT("SetMainSequenceTime Play() currentTime = %d target_sec = %d level = %s, leader = %d"), currentTime, sec, *levelName, isTeamLeader);
				FMovieSceneSequencePlaybackParams Params = FMovieSceneSequencePlaybackParams((float)sec, EUpdatePositionMethod::Jump);
				SequencePlayer->SetPlaybackPosition(Params);
				SequencePlayer->Play();
			}
			else if (currentTime != sec)
			{
				// 防止 组长掉线时 来回播放
				UE_LOG(LogTemp, Warning, TEXT("SetMainSequenceTime SequencePlayer->Pause() currentTime = %d, target_sec = %d, level = %s"), currentTime, sec, *levelName);
				SequencePlayer->Pause();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainSequencerPlayer is nullptr"));
	}
}

FSequenceInfo AInsidousPlayerBase::GetMainSequenceTime()
{
	FSequenceInfo info = FSequenceInfo();

	info.isVO = VO_Time > 30.0f;
	float t = T_Area / 100;
	info.TArea = t > 99 ? t / 100 : t;

	if (SequencePlayer)
	{	
		info.hasSequence = true;
		info.second = SequencePlayer->GetCurrentTime().AsSeconds();
	}
	info.levelFinished = LevelFinished;

	return info;
}

void AInsidousPlayerBase::ForceSetMainSequenceTime_Implementation(int32 sec)
{
	if (SequencePlayer)
	{
		auto seqTime = SequencePlayer->GetCurrentTime();
		int32 currentTime = seqTime.Time.FrameNumber.Value / seqTime.Rate.Numerator;

		currentTime = currentTime < 0 ? 0 : currentTime;

		// 获取当前 levelname
		FString levelName = UGameplayStatics::GetCurrentLevelName(this);
		// 如果是组长，则无条件 Play(), 玩家 BeginPlay 中第一次调用时，需设置 isTeamLeader 为 true
		UE_LOG(LogTemp, Warning, TEXT("ForceSetMainSequenceTime Play() currentTime = %d target_sec = %d level = %s"), currentTime, sec, *levelName);
		FMovieSceneSequencePlaybackParams Params = FMovieSceneSequencePlaybackParams((float)sec, EUpdatePositionMethod::Jump);
		SequencePlayer->SetPlaybackPosition(Params);
		SequencePlayer->Pause();
	}
}

void AInsidousPlayerBase::SetPlayerStatus_Implementation(int32 tArea, float voTime)
{
	T_Area = tArea;
	VO_Time = voTime;
}
