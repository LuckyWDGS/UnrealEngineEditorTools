// Copyright 2023 Dreamingpoet All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "IImageWrapperModule.h"
//#include "IImageWrapper.h"
//#include "ImageUtils.h"
#include "LevelSequence.h" //LevelSequence
#include "PathConfigObject.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FPathConfigData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
	    TSoftObjectPtr<UWorld> Map;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
		FVector2D MapSizeInCentimeter;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
	UTexture2D* MapImage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
	FString name;
};

/**
 * 关键帧数据结构体
 */
USTRUCT(BlueprintType)
struct FKeyFrameData
{
	GENERATED_BODY()

	/** 关键帧时间（帧数） */
	UPROPERTY(BlueprintReadOnly, Category = "LevelSequenceTools")
	int32 Frame;

	/** 关键帧值 */
	UPROPERTY(BlueprintReadOnly, Category = "LevelSequenceTools")
	int32 Value;

	FKeyFrameData()
		: Frame(0)
		, Value(0)
	{
	}
};

USTRUCT(BlueprintType, Blueprintable)
struct FTAreaInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
	FString display_name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
	int32 index;

	// in second
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InsidousLevelInfo")
	int32 duration_sec;

};

UCLASS(BlueprintType, Blueprintable)
class INSIDOUSLEVELPATH_API UPathConfigObject : public UObject
{
	GENERATED_BODY()

public:

	UPathConfigObject();
	~UPathConfigObject();
public:
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "InsidousLevelInfo")

	static void Generate(const FString& app_name, const FString& content_name, const FString& display_name, const FString& version, TArray<FPathConfigData> ConfigDat, TArray<FTAreaInfo> TAreaInfo, bool IsOpen);

	UFUNCTION(BlueprintPure, CallInEditor, Category = "InsidousLevelInfo")
	static FString GetAndroidPackageName();


	/**
	 * 通过Level Sequence路径和轨道名称获取整数轨道的关键帧数据
	 * @param LevelSequencePath		Level Sequence资源路径
	 * @param TrackName				轨道名称
	 * @param bOutSuccess			操作是否成功
	 * @param OutInfoMessage		操作结果信息
	 * @return 返回关键帧时间和值的数组
	 */
	UFUNCTION(Blueprintcallable, Category = "LevelSequenceTools")
	static TArray<FKeyFrameData> GetIntegerTrackKeyFrames(ULevelSequence* LevelSequence, FName TrackName, bool& bOutSuccess);

	UFUNCTION(Blueprintcallable, Category = "LevelSequenceTools")
	static bool GetSubsequenceByName(ULevelSequence* LevelSequence, FString SubSequenceName, ULevelSequence*& OutSubSequence, FString& OutPackagePath);


};
