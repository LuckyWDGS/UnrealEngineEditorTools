// Fill out your copyright notice in the Description page of Project Settings.


#include "InsidousBPFLibrary.h"
#include <SocketSubsystem.h>
#include "Sound/SoundWave.h" 
#include "Sound/SoundGroups.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"
#include "Json.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "UObject/Package.h"
#include "TextureResource.h"
#include "InsidousGameInstance.h"

#include <CanvasItem.h>
#include "Components/SplineComponent.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"

void UInsidousBPFLibrary::ServerTravel(UObject* WorldContextObject, FString MapName, bool bSkipNotifyPlayers /*= false*/)
{
	if (!WorldContextObject) return;
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return;
	World->ServerTravel(MapName, false, bSkipNotifyPlayers); //abs //notify players
}

FString UInsidousBPFLibrary::GetLocalIpAddress()
{
	FString ipStr("NONE");

	bool canBind = false;

#if PLATFORM_ANDROID
	ISocketSubsystem* sSS = ISocketSubsystem::Get(FName(TEXT("ANDROID")));
#else
	ISocketSubsystem* sSS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
#endif
	TSharedRef<FInternetAddr> localIP = sSS->GetLocalHostAddr(*GLog, canBind);
	// 获得本机的IP
	ipStr = (localIP->IsValid() ? localIP->ToString(false) : "");

	return ipStr;
}


FString UInsidousBPFLibrary::LoadFileToString(const FString& FilePath)
{
	FString resultString;
	FFileHelper::LoadFileToString(resultString, *FilePath);
	return resultString;
}

bool UInsidousBPFLibrary::LoadFileToBytes(const FString& FilePath, TArray<uint8>& outdata)
{
	return FFileHelper::LoadFileToArray(outdata, *FilePath);
}


void UInsidousBPFLibrary::SaveStringToFile(const FString& data, const FString& filePath, EEncodingOptions fileEncoding, bool& success)
{
	success = FFileHelper::SaveStringToFile(data, *filePath, (FFileHelper::EEncodingOptions)fileEncoding);
}


bool UInsidousBPFLibrary::IsInsidePolygon(const FVector& point, const TArray<FVector>& polygonPoints, float& minDistance)
{
	minDistance = FLT_MAX;
	// 射线法 检测某个点 Point 是否在多边形 PolygonPoints 内部
	int32 n = polygonPoints.Num();
	bool inside = false;

	if (n < 3) return false;
	
	for (int i = 0, j = n - 1; i < n; j = i++) {
        const FVector& p1 = polygonPoints[i];
		const FVector& p2 = polygonPoints[j];
		inside ^= (p1.Y > point.Y) != (p2.Y > point.Y) && (point.X < (p2.X - p1.X) * (point.Y - p1.Y) / (p2.Y - p1.Y) + p1.X);
		// Update minDistance
		FVector2D closestPoint = FMath::ClosestPointOnSegment2D(FVector2D(point), FVector2D(p1), FVector2D(p2));
		float distance = FVector2D::Distance(FVector2D(point), closestPoint);
		// 同时需要检测该点距离边线的最近垂直距离
		minDistance = FMath::Min(minDistance, distance);
    }
	// 在多边形外的话，距离0
	minDistance = inside ? minDistance : 0;
	return inside;
}

bool UInsidousBPFLibrary::RegexMatch(const FString& Str, const FString& Pattern, TArray<FString>& Result)
{
	FRegexPattern MatherPatter(Pattern);
	FRegexMatcher Matcher(MatherPatter, Str);

	while (Matcher.FindNext())
	{
		Result.Add(Matcher.GetCaptureGroup(0));

	}

	return Result.Num() == 0 ? false : true;
}


static TSharedPtr<IImageWrapper> GetImageWrapperByExtention(const FString InImagePath)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	if (InImagePath.EndsWith(".png"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	}
	else if (InImagePath.EndsWith(".jpg") || InImagePath.EndsWith(".jpeg"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	}
	else if (InImagePath.EndsWith(".bmp"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP);
	}
	else if (InImagePath.EndsWith(".ico"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::ICO);
	}
	else if (InImagePath.EndsWith(".exr"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);
	}
	else if (InImagePath.EndsWith(".icns"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::ICNS);
	}
	return nullptr;
}

static TSharedPtr<IImageWrapper> GetImageFileExtention(const FString InImagePath)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	if (InImagePath.EndsWith(".png"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	}
	else if (InImagePath.EndsWith(".jpg") || InImagePath.EndsWith(".jpeg"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	}
	else if (InImagePath.EndsWith(".bmp"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP);
	}
	else if (InImagePath.EndsWith(".ico"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::ICO);
	}
	else if (InImagePath.EndsWith(".exr"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);
	}
	else if (InImagePath.EndsWith(".icns"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::ICNS);
	}
	return nullptr;
}


//UTexture2D* UInsidousBPFLibrary::LoadTexture2DFromBytes(const TArray<uint8>& data, const FString& format, bool& IsValid, int32& OutWidth, int32& OutHeight)
//{
//	UTexture2D* Texture = nullptr;
//	IsValid = false;
//
//	TSharedPtr<IImageWrapper> ImageWrapper = GetImageWrapperByExtention("." + format);
//	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(data.GetData(), data.Num()))
//	{
//		TArray<uint8> UncompressedRGBA;
//		if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
//		{
//			Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
//			if (Texture != nullptr)
//			{
//				IsValid = true;
//				OutWidth = ImageWrapper->GetWidth();
//				OutHeight = ImageWrapper->GetHeight();
//				void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
//				FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
//				Texture->PlatformData->Mips[0].BulkData.Unlock();
//				Texture->UpdateResource();
//			}
//		}
//	}
//	return Texture;
//
//}

//UTexture2D* UInsidousBPFLibrary::LoadTexture2D(const FString& ImagePath, bool& IsValid, int32& OutWidth, int32& OutHeight)
//{
//	UTexture2D* Texture = nullptr;
//	IsValid = false;
//	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ImagePath))
//	{
//		return nullptr;
//	}
//	TArray<uint8> CompressedData;
//	if (!FFileHelper::LoadFileToArray(CompressedData, *ImagePath))
//	{
//		return nullptr;
//	}
//	TSharedPtr<IImageWrapper> ImageWrapper = GetImageWrapperByExtention(ImagePath);
//	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
//	{
//		TArray<uint8> UncompressedRGBA;
//		if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
//		{
//			Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
//			if (Texture != nullptr)
//			{
//				IsValid = true;
//				OutWidth = ImageWrapper->GetWidth();
//				OutHeight = ImageWrapper->GetHeight();
//				void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
//				FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
//				Texture->PlatformData->Mips[0].BulkData.Unlock();
//				Texture->UpdateResource();
//			}
//		}
//	}
//	return Texture;
//
//}

FString AnalysisClassName(const FString& Name)
{
	if (Name == TEXT("Actor"))
	{
		return TEXT("Blueprint");
	}

	return Name;
}

template<class T>
T* StaticLoadPakObject(const FString& Filename)
{
	const FString ObjectName = AnalysisClassName(T::StaticClass()->GetName()) + TEXT("'") + Filename + TEXT(".") + FPaths::GetCleanFilename(Filename) + TEXT("_C'");

	return Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *ObjectName));;
}

//WidgetBlueprint'/Game/ThirdPersonCPP/Blueprints/NewWidgetBlueprint.NewWidgetBlueprint'
//Blueprint'/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter.ThirdPersonCharacter'
//Material'/Game/ThirdPerson/Meshes/RampMaterial.RampMaterial'
//StaticMesh'/Game/ThirdPerson/Meshes/Ramp_StaticMesh.Ramp_StaticMesh'
template<class T>
T* StaticLoadObjectFromPak(const FString& Filename)//Filename = /Game/ThirdPerson/Meshes/RampMaterial
{
	T* ObjectInstance = StaticLoadPakObject<T>(Filename);
	//if (ObjectInstance == nullptr)
	//{
	//	RefreshAsset(FileType);

	//	if (UpdateAsset(Filename, FileType))
	//	{
	//		ObjectInstance = Cast<T>(StaticLoadPakObject<T>(Filename, Suffix));
	//	}
	//}

	return ObjectInstance;
}

bool UInsidousBPFLibrary::IsInsidousMode(UObject* WorldContextObject)
{
	if (UWorld* World = WorldContextObject->GetWorld())
	{
		UGameInstance* GameInstance = World->GetGameInstance();
		UInsidousGameInstance* GI = Cast<UInsidousGameInstance>(GameInstance);
		if (GI && GI->SocketObject) return true;
	}
	return false;
}

UClass* UInsidousBPFLibrary::StaticLoadClass(const FString& Reference)
{
	return StaticLoadObjectFromPak<UClass>(Reference);
}

USoundWave* UInsidousBPFLibrary::LoadSoundWaveFromPak(const FString& Reference)
{
	return Cast<USoundWave>(StaticLoadObject(USoundWave::StaticClass(), nullptr, *Reference));
}

UObject* UInsidousBPFLibrary::LoadObject(const FString& Reference)
{
	return Cast<UObject>(StaticLoadObject(UObject::StaticClass(), nullptr, *Reference));
}


FDateTime UInsidousBPFLibrary::Add_DateTimeTimespan(FDateTime A, FTimespan B)
{
	return A + B;
}


FDateTime UInsidousBPFLibrary::Subtract_DateTimeTimespan(FDateTime A, FTimespan B)
{
	return A - B;
}


void UInsidousBPFLibrary::AsyncLevelLoad(UObject* WorldContextObject, const FString& LevelPath, const bool OpenWhenLoaded)
{

	LoadPackageAsync(LevelPath,
		FLoadPackageAsyncDelegate::CreateLambda([WorldContextObject, LevelPath, OpenWhenLoaded](const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
			{
				if (Result == EAsyncLoadingResult::Succeeded && OpenWhenLoaded)
				{
					UGameplayStatics::OpenLevel(WorldContextObject, FName(*(LevelPath)));
				}

			}
		),
		0,
		PKG_ContainsMap);
}
void UInsidousBPFLibrary::CreateWallMesh(const TArray<FVector>& BasePoints, float height, TArray<FVector>& OutVertexs, TArray<int32>& OutTriangles, TArray<FVector2D>& OutUVs)
{
	FVector PrevPoint = FVector(0.f);
	float TotalLength = 0.f;
	float currentUV = 0.f;
	TArray<float> Lengths;


	//获取每两个点之间距离，并计算总长，然后计算每个点距离起始点的距离，再除总长获得比例
	for (int32 i = 0; i < BasePoints.Num(); ++i)
	{
		if( i == 0)
		{
			PrevPoint = BasePoints[i];
			Lengths.Add(0.f);
		} else 
		{
			TotalLength += FVector::Distance(BasePoints[i], PrevPoint);
			Lengths.Add(TotalLength);
			PrevPoint = BasePoints[i];
		}
	}

	for(int32 i = 0; i < BasePoints.Num(); ++i)
	{
		FVector current = BasePoints[i];
		OutVertexs.Add(FVector(current.X, current.Y, 0.f));
		OutVertexs.Add(FVector(current.X, current.Y, height));

		OutTriangles.Add(2 * i);
		OutTriangles.Add(2 * i + 2);
		OutTriangles.Add(2 * i + 3);

		OutTriangles.Add(2 * i);
		OutTriangles.Add(2 * i + 3);
		OutTriangles.Add(2 * i + 1);

		currentUV = Lengths[i] / height;

		OutUVs.Add(FVector2D(currentUV, 0.f));
		OutUVs.Add(FVector2D(currentUV, 1.f));
	}
}

FString UInsidousBPFLibrary::GetCurrentLevelNameWithPath(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return FString();
	}
	
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return FString();
	}
	
	// 获取当前关卡
	ULevel* CurrentLevel = World->GetCurrentLevel();
	if (!CurrentLevel)
	{
		return FString();
	}
	
	// 获取关卡包的路径
	UPackage* LevelPackage = CurrentLevel->GetOutermost();
	if (!LevelPackage)
	{
		return FString();
	}

	// 获取完整路径名
	FString LevelPath = LevelPackage->GetName();
	
	// 如果是PIE（Play In Editor）模式，需要移除PIE前缀
	if (World->WorldType == EWorldType::PIE)
	{
		FString PIEPrefix = World->StreamingLevelsPrefix;
		if (!PIEPrefix.IsEmpty())
		{
			LevelPath = LevelPath.Replace(*PIEPrefix, TEXT(""));
		}
	}
	
	return LevelPath;
}

void UInsidousBPFLibrary::AsyncLevelLoadByObjectReference(UObject* WorldContextObject, TSoftObjectPtr<UWorld> WorldSoftObject, const bool OpenWhenLoaded)
{
	FString LevelPath = WorldSoftObject.ToString();

	LoadPackageAsync(LevelPath,
		FLoadPackageAsyncDelegate::CreateLambda([WorldContextObject, WorldSoftObject, OpenWhenLoaded](const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
			{
				if (Result == EAsyncLoadingResult::Succeeded && OpenWhenLoaded)
				{
					UGameplayStatics::OpenLevelBySoftObjectPtr(WorldContextObject, WorldSoftObject);
				}
			}
		),
		0,
		PKG_ContainsMap);
}

UAsyncLoadLevel* UAsyncLoadLevel::AsyncLoadLevel(UObject* WorldContextObject, TSoftObjectPtr<UWorld> WorldSoftObject, const FString& LevelPath, const bool OpenWhenLoaded)
{
	UAsyncLoadLevel* Node = NewObject<UAsyncLoadLevel>(WorldContextObject);
	Node->WorldSoftObject = WorldSoftObject;
	Node->LevelPath = LevelPath;
	Node->OpenWhenLoaded = OpenWhenLoaded;
	Node->WorldContextObject = WorldContextObject;
	return Node;
}

void UAsyncLoadLevel::OnGetReturn(const bool isLoaded)
{
	if(isLoaded)
		OnCompleted.Broadcast();
	else
		OnFailed.Broadcast();	
}

void UAsyncLoadLevel::Activate()
{
	FString TargetLevelPath = WorldSoftObject.IsNull() ? this->LevelPath: WorldSoftObject.ToString();

	LoadPackageAsync(TargetLevelPath,
		FLoadPackageAsyncDelegate::CreateLambda([this, TargetLevelPath](const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
			{
				this->OnGetReturn(Result == EAsyncLoadingResult::Succeeded);
				if (Result == EAsyncLoadingResult::Succeeded && this->OpenWhenLoaded)
				{
					UGameplayStatics::OpenLevel(this->WorldContextObject, FName(*(TargetLevelPath)));
				}
			}
		),
		0,
		PKG_ContainsMap);
}

FVector2D UInsidousBPFLibrary::K2_DrawSmoothSpline3DTo2D(UCanvas* Canvas, USplineComponent* SplineComponent, FTransform RelativeTransform, FVector2D BoundsOffset, FVector2D Resolution, float Thickness, FLinearColor RenderColor, float Smoothness)
{
	// 增加更严格的空指针检查
	if (!Canvas || !IsValid(Canvas) || !SplineComponent || !IsValid(SplineComponent))
	{
		return FVector2D::ZeroVector;
	}

	// 检查分辨率
	if (Resolution.X <= 0 || Resolution.Y <= 0)
	{
		return FVector2D::ZeroVector;
	}

	// 检查样条线组件是否有效
	if (!SplineComponent->IsValidLowLevel() || !SplineComponent->IsRegistered())
	{
		return FVector2D::ZeroVector;
	}

	int32 NumSplinePoints = SplineComponent->GetNumberOfSplinePoints();
	if (NumSplinePoints < 2)
	{
		return FVector2D::ZeroVector;
	}

	// 计算总长度
	float TotalLength = SplineComponent->GetSplineLength();
	if (TotalLength <= 0.0f)
	{
		return FVector2D::ZeroVector;
	}

	// 使用更多的点来采样样条线
	// 公式：总点数 = (总长度 / 距离区间) * 每区间点数
	int32 NumSamples = FMath::CeilToInt((TotalLength / 100) * 50);
	float StepSize = TotalLength / NumSamples;

	FVector2D ScreenCenter = Resolution / 2.0f;
	FVector2D PreviousScreenPos = FVector2D(10000, 10000);

	// 用于计算中心点的变量
	FVector2D MinScreenPos(FLT_MAX, FLT_MAX);
	FVector2D MaxScreenPos(-FLT_MAX, -FLT_MAX);
	FVector2D SumScreenPos = FVector2D::ZeroVector;
	int32 ValidPoints = 0;

	// 使用更多的点来创建更平滑的曲线
	// 采样样条线上的点
	for (int32 i = 0; i <= NumSamples; ++i)
	{
		float Distance = i * StepSize;

		// 获取当前位置和切线
		FVector CurrentWorldPos3D = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		//FVector Tangent = SplineComponent->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

					// 将世界坐标转换为相对于 RelativeTrans 的局部坐标
		FVector Offset = RelativeTransform.InverseTransformPosition(CurrentWorldPos3D);

		// 应用平滑度
		float SmoothFactor = FMath::Clamp(Smoothness, 0.0f, 1.0f);
		FVector2D CurrentWorldPos(Offset.X, Offset.Y);

		//// 计算屏幕位置
		//FVector2D CurrentWorldOffset = CurrentWorldPos - FVector2D(RelativeTransform.GetLocation().X, RelativeTransform.GetLocation().Y);
		FVector2D CurrentScreenPos = ScreenCenter + (CurrentWorldPos - BoundsOffset);

		// 更新边界框
		MinScreenPos.X = FMath::Min(MinScreenPos.X, CurrentScreenPos.X);
		MinScreenPos.Y = FMath::Min(MinScreenPos.Y, CurrentScreenPos.Y);
		MaxScreenPos.X = FMath::Max(MaxScreenPos.X, CurrentScreenPos.X);
		MaxScreenPos.Y = FMath::Max(MaxScreenPos.Y, CurrentScreenPos.Y);

		// 累加有效点的位置
		if (CurrentScreenPos.X >= 0 && CurrentScreenPos.X < Resolution.X &&
			CurrentScreenPos.Y >= 0 && CurrentScreenPos.Y < Resolution.Y)
		{
			SumScreenPos += CurrentScreenPos;
			ValidPoints++;
		}

		// 检查点是否在屏幕范围内
		bool bCurrentInBounds = CurrentScreenPos.X >= 0 && CurrentScreenPos.X < Resolution.X &&
			CurrentScreenPos.Y >= 0 && CurrentScreenPos.Y < Resolution.Y;

		if (bCurrentInBounds && PreviousScreenPos != FVector2D(10000, 10000))
		{
			// 绘制线段
			FCanvasLineItem LineItem(PreviousScreenPos, CurrentScreenPos);
			LineItem.LineThickness = Thickness;
			LineItem.SetColor(RenderColor);
			Canvas->DrawItem(LineItem);
		}
		PreviousScreenPos = CurrentScreenPos;
	}

	// 计算并返回中心点
	if (ValidPoints > 0)
	{
		return SumScreenPos / ValidPoints;
	}
	else
	{
		// 如果没有有效点，返回边界框的中心点
		return (MinScreenPos + MaxScreenPos) * 0.5f;
	}
}

FVector2D UInsidousBPFLibrary::K2_CalculateSplinesBounds(const TArray<USplineComponent*>& SplineComponents, FTransform RelativeTransform, FVector2D& BoundsOffset)
{
	if (SplineComponents.Num() == 0)
	{
		return FVector2D(100.0f, 100.0f); // 返回默认分辨率
	}

	// 初始化边界值
	float MinX = FLT_MAX;
	float MaxX = -FLT_MAX;
	float MinY = FLT_MAX;
	float MaxY = -FLT_MAX;

	// 遍历所有样条线
	for (USplineComponent* SplineComponent : SplineComponents)
	{
		if (!SplineComponent || !IsValid(SplineComponent))
		{
			continue;
		}

		int32 NumSplinePoints = SplineComponent->GetNumberOfSplinePoints();
		if (NumSplinePoints < 2)
		{
			continue;
		}

		// 计算样条线的总长度
		float TotalLength = SplineComponent->GetSplineLength();
		if (TotalLength <= 0.0f)
		{
			continue;
		}

		// 使用更多的点来采样样条线
	   // 公式：总点数 = (总长度 / 距离区间) * 每区间点数
		int32 NumSamples = FMath::CeilToInt((TotalLength / 100) * 50);
		float StepSize = TotalLength / NumSamples;

		// 采样样条线上的点
		for (int32 i = 0; i <= NumSamples; ++i)
		{
			float Distance = i * StepSize;
			FVector WorldPos = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

			//// 计算相对于中心点的偏移
			//FVector Offset = WorldPos - CenterWorldPos;

			// 将世界坐标转换为相对于 RelativeTrans 的局部坐标
			FVector Offset = RelativeTransform.InverseTransformPosition(WorldPos);


			// 更新边界
			MinX = FMath::Min(MinX, Offset.X);
			MaxX = FMath::Max(MaxX, Offset.X);
			MinY = FMath::Min(MinY, Offset.Y);
			MaxY = FMath::Max(MaxY, Offset.Y);
		}

		// 检查样条点的切线方向，确保曲线不会超出边界
		for (int32 i = 0; i < NumSplinePoints; ++i)
		{
			FVector Tangent = SplineComponent->GetDirectionAtSplinePoint(i, ESplineCoordinateSpace::World);
			FVector WorldPos = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
			//FVector Offset = WorldPos - CenterWorldPos;

			// 将世界坐标转换为相对于 RelativeTrans 的局部坐标
			FVector Offset = RelativeTransform.InverseTransformPosition(WorldPos);

			// 根据切线方向扩展边界
			float TangentScale = 10.0f; // 切线方向的扩展量
			MinX = FMath::Min(MinX, Offset.X - Tangent.X * TangentScale);
			MaxX = FMath::Max(MaxX, Offset.X + Tangent.X * TangentScale);
			MinY = FMath::Min(MinY, Offset.Y - Tangent.Y * TangentScale);
			MaxY = FMath::Max(MaxY, Offset.Y + Tangent.Y * TangentScale);
		}
	}

	// 计算边界大小
	float Width = (MaxX - MinX);
	float Height = (MaxY - MinY);

	// 添加边距（使用百分比而不是固定值）
	const float PaddingPercent = 0.01f; // 10%的边距
	Width *= (1.0f + PaddingPercent * 2.0f);
	Height *= (1.0f + PaddingPercent * 2.0f);

	// 确保最小尺寸
	Width = FMath::Max(Width, 100.0f);
	Height = FMath::Max(Height, 100.0f);

	// 确保宽高比为整数
	Width = FMath::CeilToFloat(Width);
	Height = FMath::CeilToFloat(Height);
	BoundsOffset = FVector2D((MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f);
	return FVector2D(Width, Height);
}

bool UInsidousBPFLibrary::K2_SaveRenderTargetToPNG(UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget, const FString& SavePath)
{
	// 检查保存路径是否有效
	if (SavePath.IsEmpty())
	{
		return false;
	}

	if (RenderTarget)
	{

		//获取randerTarget贴图资源  将颜色值全部放入FTextureRenderTargetResource中
		FTextureRenderTargetResource* TextureRenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
		int32 Width = RenderTarget->SizeX;//获取高度
		int32 Height = RenderTarget->SizeY;//获取宽度
		TArray<FColor> OutData;//声明一个Fcolor数组
		TextureRenderTargetResource->ReadPixels(OutData);  //读取像素点
		for (FColor& Color : OutData)
		{
			if (Color.A == 0) {
				uint8 MaxComponent = FMath::Max3(Color.R, Color.G, Color.B);
				Color.A = MaxComponent;
			}
		}
		ColorToImage(SavePath, OutData, Width, Height);//写入到本地存成图片
		return true;

	}
	return false;
}

void UInsidousBPFLibrary::ColorToImage(const FString& InImagePath, TArray<FColor> InColor, int32 InWidth, int32 InHight)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	FString	Ex = FPaths::GetExtension(InImagePath);
	if (Ex.Equals(TEXT("jpg"), ESearchCase::IgnoreCase) || Ex.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase))
	{
		TSharedPtr<IImageWrapper>ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		//往本地写，获取颜色数据，获取尺寸，获取长度，高度，格式rgb,8位
		if (ImageWrapper->SetRaw(InColor.GetData(), InColor.GetAllocatedSize(), InWidth, InHight, ERGBFormat::BGRA, 8))
		{
			FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(80), *InImagePath);
		}
	}
	else
	{
		TArray64<uint8>OutPNG;
		// 保持原始颜色的A通道值
		FImageUtils::PNGCompressImageArray(InWidth, InHight, InColor, OutPNG);
		FFileHelper::SaveArrayToFile(OutPNG, *InImagePath);
	}
}

void UInsidousBPFLibrary::K2_DrawTextAtSplineCenter(UCanvas* Canvas, FVector2D CenterScreenPos, const FString& Text, float TextSize, FLinearColor TextColor, UFont* Font)
{
	// 增加更严格的空指针检查
	if (!IsValid(Canvas))
	{
		return;
	}

	// 检查分辨率
	if (CenterScreenPos.X <= 0 || CenterScreenPos.Y <= 0)
	{
		return;
	}
	if (TextColor.A == 0)
	{
		TextColor.A = 1;
	}
	// 绘制文字
	if (!Text.IsEmpty())
	{
		// 设置文字属性
		FCanvasTextItem TextItem(CenterScreenPos, FText::FromString(Text), Font ? Font : GEngine->GetMediumFont(), TextColor);
		TextItem.Scale = FVector2D(TextSize, TextSize);
		TextItem.bCentreX = true;  // 水平居中
		TextItem.bCentreY = true;  // 垂直居中

		// 绘制文字
		Canvas->DrawItem(TextItem);
	}
}


//void UInsidousBPFLibrary::GameSceneShot(USceneCaptureComponent2D* CaptureComponent2D, const FString& SavePath)
//{
//	if (CaptureComponent2D && CaptureComponent2D->TextureTarget) {
//		auto Lab = [=]()
//			{
//				FTextureRenderTargetResource* TextureRenderTargetResource = CaptureComponent2D->TextureTarget->GameThread_GetRenderTargetResource();
//				int32 Width = CaptureComponent2D->TextureTarget->SizeX;
//				int32 Height = CaptureComponent2D->TextureTarget->SizeY;
//				TArray<FColor> OutData;
//				TextureRenderTargetResource->ReadPixels(OutData);
//
//				ColorToImage(SavePath, OutData, Width, Height);
//			};
//
//		FTimerHandle TimerHandle;
//		GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World()->GetTimerManager().SetTimer(TimerHandle, Lab, 0.001f, false, 0);
//		return;
//	}
//
//	UE_LOG(LogTemp, Error, TEXT("CaptureComponent2D == Null Or CaptureComponent2D->TextureTarget == Null "));
//}


TArray<uint8> UInsidousBPFLibrary::GameSceneShotToArrayJPEG(USceneCaptureComponent2D* CaptureComponent2D)
{
	TArray<uint8> DestArray;
	if (CaptureComponent2D && CaptureComponent2D->TextureTarget) {
		

		FTextureRenderTargetResource* TextureRenderTargetResource = CaptureComponent2D->TextureTarget->GameThread_GetRenderTargetResource();
		int32 Width = CaptureComponent2D->TextureTarget->SizeX;
		int32 Height = CaptureComponent2D->TextureTarget->SizeY;
		TArray<FColor> OutData;
		TextureRenderTargetResource->ReadPixels(OutData);

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");

		TSharedPtr<IImageWrapper>ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		//往本地写，获取颜色数据，获取尺寸，获取长度，高度，格式rgb,8位
		if (ImageWrapper->SetRaw(OutData.GetData(), OutData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8))
		{
			auto res =  ImageWrapper->GetCompressed(80);
			DestArray.SetNumUninitialized(res.Num());
			FMemory::Memcpy(DestArray.GetData(), res.GetData(), res.Num());
		}
	}
	return DestArray;
}

void UInsidousBPFLibrary::CaptureSceneDeferred(USceneCaptureComponent2D* CaptureComponent2D)
{
	if(CaptureComponent2D != nullptr)
	{
		CaptureComponent2D->CaptureSceneDeferred();
	}
}


//
//
//#if WITH_EDITOR
//#pragma optimize("", off)
//#endif
//
//#if WITH_EDITOR
//#pragma optimize("", on)
//#endif
