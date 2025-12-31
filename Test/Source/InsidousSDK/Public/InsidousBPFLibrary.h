// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
//#include "SplineToolsType.h"
#include "Components/SplineComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/Canvas.h"

#include "InsidousBPFLibrary.generated.h"

UENUM(BlueprintType)
enum class EEncodingOptions : uint8
{
	AutoDetect,
	ForceAnsi,
	ForceUnicode,
	ForceUTF8,
	ForceUTF8WithoutBOM
};

UCLASS()
class INSIDOUSSDK_API UInsidousBPFLibrary : public UBlueprintFunctionLibrary 
{
	GENERATED_BODY()

public:
		

/** Server Travel! This is an async load level process which allows you to put up a UMG widget while the level loading occurs! */
UFUNCTION(BlueprintCallable, Category = "Insdous BP Library", meta = (WorldContext = "WorldContextObject"))
	static void ServerTravel(UObject* WorldContextObject, FString MapName, bool bSkipNotifyPlayers = false);

// 是否是在编辑器模式下
// 尽量不用，除非用来初始化一些需要运行时获取的数据，不利于在编辑器下的测试
UFUNCTION(BlueprintPure, Category = "Insdous BP Library")
	static bool IsWithEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

	// 是否是安卓
	UFUNCTION(BlueprintPure, Category = "Insdous BP Library")
	static bool IsAndroid()
	{
#if PLATFORM_ANDROID
		return true;
#else
		return false;
#endif
	}

// 是否是Insidous 部署模式
UFUNCTION(BlueprintPure, Category = "Insdous BP Library", meta = (WorldContext = "WorldContextObject"))
static bool IsInsidousMode(UObject* WorldContextObject);

// Reference = /Game/ThirdPerson/Meshes/RampMaterial
UFUNCTION(BlueprintPure, Category = "Insdous BP Library|StaticLoad")
	static UClass* StaticLoadClass(const FString& Reference);

// Reference = SoundWave'/Game/ThirdPerson/Meshes/Sopund.Sopund'
UFUNCTION(BlueprintPure, Category = "Insdous BP Library|StaticLoad")
	static USoundWave* LoadSoundWaveFromPak(const FString& Reference);


// Reference = SoundWave'/Game/ThirdPerson/Meshes/Sopund.Sopund'
UFUNCTION(BlueprintPure, Category = "Insdous BP Library|StaticLoad")
	static UObject* LoadObject(const FString& Reference);

// 获取本地IP
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Insdous BP Library|NetServer")
static FString GetLocalIpAddress();

UFUNCTION(BlueprintCallable, Category = "Insdous BP Library|Files")
static FString LoadFileToString(const FString& FilePath);

UFUNCTION(BlueprintCallable, Category = "Insdous BP Library|Files")
static bool LoadFileToBytes(const FString& FilePath, TArray<uint8>& outdata);

UFUNCTION(BlueprintCallable, Category = "Insdous BP Library|Files")
static void SaveStringToFile(const FString& data, const FString& filePath, EEncodingOptions fileEncoding, bool& success);

/**
 * 计算一个点是否在多边形中
 *
 * @param point  要计算的点
 * @param PolygonPoints  多边形的顶点必须是按顺序给出的
*/
UFUNCTION(BlueprintCallable, Category = "Insdous BP Library|MathHelper")
static bool IsInsidePolygon(const FVector& point, const TArray<FVector>& polygonPoints, float& minDistance);

UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Str with RegularExpression", Keywords = "RegularExpression sample test testing"), Category = "Insdous BP Library|RegularExpression")
static bool RegexMatch(const FString& Str, const FString& Pattern, TArray<FString>& Result);

/** Addition (A + B) */
UFUNCTION(BlueprintCallable,  Category = "Insidous|Math|DateTime")
static FDateTime Add_DateTimeTimespan(FDateTime A, FTimespan B);

/** Subtraction (A - B) */
UFUNCTION(BlueprintCallable, Category = "Insidous|Math|DateTime")
static FDateTime Subtract_DateTimeTimespan(FDateTime A, FTimespan B);

UFUNCTION(BlueprintCallable, Category = "Insdous BP Library", meta = (WorldContext = "WorldContextObject"))
static void AsyncLevelLoadByObjectReference(UObject* WorldContextObject, TSoftObjectPtr<UWorld> WorldSoftObject, const bool OpenWhenLoaded);

/** LevelPath /Game/xxxmap */
UFUNCTION(BlueprintCallable, Category = "Insdous BP Library", meta = (WorldContext = "WorldContextObject"))
static void AsyncLevelLoad(UObject* WorldContextObject, const FString& LevelPath, const bool OpenWhenLoaded);

UFUNCTION(BlueprintCallable, Category = "Insidous|Math|DateTime")
static void CreateWallMesh(const TArray<FVector>& BasePoints,float height, TArray<FVector>& OutVertexs, TArray<int32>& OutTriangles, TArray<FVector2D>& OutUVs);

UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Insidous", meta = (WorldContext = "WorldContextObject"))
static FString GetCurrentLevelNameWithPath(UObject* WorldContextObject);



/* 黄蓝框导出功能  ---------------------------------------------------------------------*/

UFUNCTION(BlueprintCallable, Category = Canvas, meta = (DisplayName = "Draw Smooth Spline 3D to 2D", ScriptName = "DrawSmoothSpline3DTo2D"))
static FVector2D K2_DrawSmoothSpline3DTo2D(UCanvas* Canvas, USplineComponent* SplineComponent, FTransform RelativeTransform, FVector2D BoundsOffset, FVector2D Resolution, float Thickness = 1.0f, FLinearColor RenderColor = FLinearColor::White, float Smoothness = 0.5f);

UFUNCTION(BlueprintCallable, Category = "SplineTools|计算", meta = (DisplayName = "Calculate Splines Bounds", ScriptName = "CalculateSplinesBounds"))
static FVector2D K2_CalculateSplinesBounds(const TArray<USplineComponent*>& SplineComponents, FTransform RelativeTransform, FVector2D& BoundsOffset);

UFUNCTION(BlueprintCallable, Category = "SplineTools|计算", meta = (DisplayName = "Draw Text At Spline Center", ScriptName = "DrawTextAtSplineCenter"))
static void K2_DrawTextAtSplineCenter(UCanvas* Canvas, FVector2D CenterScreenPos, const FString& Text, float TextSize, FLinearColor TextColor, UFont* Font);

UFUNCTION(BlueprintCallable, Category = "SplineTools|保存", meta = (WorldContext = "WorldContextObject", DisplayName = "Save Render Target To PNG", ScriptName = "SaveRenderTargetToPNG"))
static bool K2_SaveRenderTargetToPNG(UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget, const FString& SavePath);
static void ColorToImage(const FString& InImagePath, TArray<FColor>InColor, int32 InWidth, int32 InHight);//将颜色数据提取

/* 黄蓝框导出功能  ---------------------------------------------------------------------*/


/* 场景截图并且保存到目录
  *  @param		CaptureComponent2D		需要指定一个UE 2D相机
  *  @param		SavePath				截图需要保存的路径
  */
//UFUNCTION(BlueprintCallable, Category = "ExtendedContent|ScreenShot")
//static void GameSceneShot(class USceneCaptureComponent2D* CaptureComponent2D, const FString& SavePath);


/* 场景截图并且保存到目录
  *  @param		CaptureComponent2D		需要指定一个UE 2D相机
  *  @param		data					二进制数据
  */
UFUNCTION(BlueprintCallable, Category = "ExtendedContent|ScreenShot")
static TArray<uint8> GameSceneShotToArrayJPEG(class USceneCaptureComponent2D* CaptureComponent2D);


/* 场景截图并且保存到目录
  *  @param		CaptureComponent2D		需要指定一个UE 2D相机
  *  @param		data					二进制数据
  */
UFUNCTION(BlueprintCallable, Category = "ExtendedContent|ScreenShot")
static void CaptureSceneDeferred(class USceneCaptureComponent2D* CaptureComponent2D);



};



#pragma region UAsyncLoadLevel

UCLASS()
class INSIDOUSSDK_API UAsyncLoadLevel : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	// 返回结果代理
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncLoadLevelResult);

	UPROPERTY(BlueprintAssignable)
	FAsyncLoadLevelResult OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FAsyncLoadLevelResult OnFailed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Category = "Insidous Async Node"))
	static UAsyncLoadLevel* AsyncLoadLevel(UObject* WorldContextObject, TSoftObjectPtr<UWorld> WorldSoftObject, const FString& LevelPath, const bool OpenWhenLoaded);

	// 当获取到返回值的时候
	void OnGetReturn(const bool isLoaded);


protected:
	UObject* WorldContextObject;
	TSoftObjectPtr<UWorld> WorldSoftObject;
	FString LevelPath;
	bool OpenWhenLoaded = false;

	void Activate() override;

};
#pragma endregion
