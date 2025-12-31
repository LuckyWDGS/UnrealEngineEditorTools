// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InsidousPlayerInterface.h"
#include "Engine/TimerHandle.h"
#include "InsidousPlayerBase.generated.h"


class ULevelSequencePlayer;
class AInsidousLevelManager;

UCLASS()
class INSIDOUSSDK_API AInsidousPlayerBase : public APawn, public IInsidousPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AInsidousPlayerBase();

	FTimerHandle GetOtherActorsTimeHandle;

	// 当前载具，需要在Sequencer 中设置和修改
	UPROPERTY(BlueprintReadOnly, Interp, Category = "Insidous Player")
	AActor* Vehicle;

	UPROPERTY(BlueprintReadOnly, Category = "Insidous Player")
	ULevelSequencePlayer* SequencePlayer;

	UPROPERTY(BlueprintReadOnly, Category = "Insidous Player")
	AActor* LevelManagerActor;

	UPROPERTY(BlueprintReadOnly, Interp, Category = "Insidous Player")
	bool LevelFinished;

	UPROPERTY(BlueprintReadOnly, Interp, Category = "Insidous Player")
	AActor* AttachedVehicle;

	UFUNCTION()
	virtual void OnMainSequencerFinished();

	UFUNCTION(BlueprintCallable, Category = "Insidous Player")
	FSequenceInfo GetMainSequenceTime();

	// 实现 InsidouPlayerInterface

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
	void SetMainSequenceTime(int32 sec, bool isTeamLeader);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
	void ForceSetMainSequenceTime(int32 sec);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InsidousPlayerInterface")
	void SetPlayerStatus(int32 TArea, float VOTime);
	// 实现 InsidouPlayerInterface

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Insidous Player")
	AActor* TryGetLevelManagerActor();

	UFUNCTION(BlueprintCallable, Category = "Insidous Player")
	void TryGetMainSequencerPlayer();

	UFUNCTION(BlueprintCallable, Category = "Insidous Player")
	void InsidousPause(bool Pause);

private:

	int32 T_Area;
	float VO_Time;
	bool ManualPaused = false;
};
