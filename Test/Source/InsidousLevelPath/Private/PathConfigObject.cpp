// Copyright 2023 Dreamingpoet All Rights Reserved.

#include "PathConfigObject.h"
#include "Engine/TextureRenderTarget2D.h"
#include <ImageUtils.h>
#include "Json.h"
//#include "Misc/FileHelper.h"
#include "AndroidRuntimeSettings.h"

#include "Misc/AES.h"
#include "Misc/Base64.h"


#include "Channels/MovieSceneChannelProxy.h"//MovieScene
#include <Sections/MovieSceneIntegerSection.h>
#include <Sections/MovieSceneSubSection.h>
#include <Tracks/MovieSceneIntegerTrack.h>
#include <Tracks/MovieSceneSubTrack.h>
#include "MovieScene.h" // Moviescene
#include "MovieScenesection.h" // Moviescene

#if WITH_SSL
#define UI UI_ST

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#undef UI
#endif // WITH_SSL

//#if PLATFORM_ANDROID
//#include <openssl/evp.h>
//#include <openssl/aes.h>
//#include <openssl/err.h>
//#else
//#include "ThirdParty/OpenSSLLib/1.1.1t/include/Win64/VS2015/openssl/evp.h"
//#include "ThirdParty/OpenSSLLib/1.1.1t/include/Win64/VS2015/openssl/aes.h"
//#include "ThirdParty/OpenSSLLib/1.1.1t/include/Win64/VS2015/openssl/err.h"
//#endif

UPathConfigObject::UPathConfigObject() : Super() {}


UPathConfigObject::~UPathConfigObject() {}


#if WITH_EDITOR
#pragma optimize("", off)
#endif

FString Texture2Base64String(UTexture2D* Texture)
{
	
	//  ** Texture To FColor ** 
	TArray<FColor> colors;
	TArray<FColor> dstColors;
	TextureCompressionSettings OldCompressionSettings = Texture->CompressionSettings;
	TextureMipGenSettings OldMipGenSettings = Texture->MipGenSettings;
	bool OldSRGB = Texture->SRGB;

	// modified to exportable settings
	Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	Texture->SRGB = false;
	Texture->UpdateResource();

	// export texture to image
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	uint8* TextureData = (uint8*)Mip.BulkData.Lock(LOCK_READ_WRITE);

	int32 SizeX = Texture->GetPlatformData()->SizeX;
	int32 SizeY = Texture->GetPlatformData()->SizeY;
	
	for (int32 IndexY = 0; IndexY < SizeY; IndexY++)
	{
		for (int32 IndexX = 0; IndexX < SizeX; IndexX++)
		{  
			int32 pixelIndex = (IndexY * SizeX + IndexX) * 4;
			FColor PixelColor;
			PixelColor.B = TextureData[pixelIndex + 0];
			PixelColor.G = TextureData[pixelIndex + 1];
			PixelColor.R = TextureData[pixelIndex + 2];
			//PixelColor.A = PixelColor.B+ PixelColor.G+ PixelColor.R == 0 ? 0 : 1;
			PixelColor.A= TextureData[pixelIndex + 3];
			colors.Add(PixelColor);
		}
	}
	Mip.BulkData.Unlock();
	//  ** Texture To FColor ** 

	//  ** FColor To Base64String ** 
	TArray64<uint8> CompressedBitmap;
	float minwidth=512.0f;
	float scaleX = minwidth / SizeX;
	float scaleY = minwidth / SizeY;
	float scale = FMath::Max(scaleX, scaleY);
	float NewSizeX = FMath::CeilToInt(SizeX * scale);
	float NewSizeY = FMath::CeilToInt(SizeY * scale);
	
	FImageUtils::ImageResize( SizeX, SizeY, colors, NewSizeX, NewSizeY, dstColors, true, false);
	FImageUtils::PNGCompressImageArray(NewSizeX, NewSizeY, dstColors, CompressedBitmap);
	FString base64String = TEXT("data:image/png;base64,") + FBase64::Encode(CompressedBitmap.GetData(), CompressedBitmap.Num());
	return base64String;
	// UE_LOG(LogTemp, Warning, TEXT("base64 string = ,%s"), *base64String);
	//  ** FColor To Base64String ** 
}

FString GenKey()
{
	TArray<uint8> KeyBytes;
	// 原来的 ajV5Vm53WDZ3RVR4djc4SjRrdlo5QUx4d1UwNnU1SlA5Z0pwT201b1luZ2l1RVRWSw==
	FBase64::Decode("ajV5Vm53WDZ3RVR4djc4SjRrdlo5QUx4d1UwNnU1SlA5Z0pwT201b1luZ2l1RVRWSw==", KeyBytes);
	FString KeyString = FString(UTF8_TO_TCHAR(KeyBytes.GetData()));
	KeyString.RightChopInline(9);
	FString Left1 = KeyString.Left(17);
	KeyString.RightChopInline(25);
	FString Right = KeyString.Left(15);
	return Left1 + Right;
}

FString AESEncrypt(const FString& data, const FString& key)
{
#if WITH_SSL

	// Convert FString to byte array
	TArray<uint8> DataBytes;
	FTCHARToUTF8 Convert(*data);
	DataBytes.Append((uint8*)Convert.Get(), Convert.Length());

	// Generate a random nonce
	uint8 Nonce[12];
	FGuid guid;
	FPlatformMisc::CreateGuid(guid);
	FString GuidString = guid.ToString().Left(12);
	FTCHARToUTF8 ConvertGuid(*GuidString);
	FMemory::Memcpy(Nonce, ConvertGuid.Get(), 12);

	// Prepare the encryption context
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(Nonce), NULL);
	EVP_EncryptInit_ex(ctx, NULL, NULL, (uint8*)TCHAR_TO_UTF8(*key), Nonce);

	// Encrypt the data
	TArray<uint8> Ciphertext;
	Ciphertext.SetNum(DataBytes.Num() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
	int OutLen;
	EVP_EncryptUpdate(ctx, Ciphertext.GetData(), &OutLen, DataBytes.GetData(), DataBytes.Num());

	// Finalize encryption
	int FinalLen;
	EVP_EncryptFinal_ex(ctx, Ciphertext.GetData() + OutLen, &FinalLen);
	Ciphertext.SetNum(OutLen + FinalLen);

	// Get the tag
	uint8 Tag[16];
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(Tag), Tag);


	// Append the tag to the ciphertext
	Ciphertext.Append(Tag, sizeof(Tag));

	// Append the Nonce to the ciphertext
	Ciphertext.Append(Nonce, sizeof(Nonce));

	// Clean up
	EVP_CIPHER_CTX_free(ctx);

	// Convert to Base64 for easy storage
	FString EncryptedData = FBase64::Encode(Ciphertext);
	return EncryptedData;
#endif // WITH_SSL
//	return FString();
}

void UPathConfigObject::Generate(const FString& app_name, const FString& content_name, const FString& display_name, const FString& version, TArray<FPathConfigData> ConfigData, TArray<FTAreaInfo> TAreaInfo, bool IsOpen)
{
	FString uuid = FGuid::NewGuid().ToString();
	FString JsonStr;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonStr);

	//{ (main)
	JsonWriter->WriteObjectStart();

	JsonWriter->WriteValue("uid", uuid);
	JsonWriter->WriteValue("app_name", app_name);
	JsonWriter->WriteValue("content_name", content_name);
	JsonWriter->WriteValue("display_name", display_name);
	JsonWriter->WriteValue("version", version);
	// UnrealEngine 获取当前时间，并格式化
	JsonWriter->WriteValue("create_time", FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")));

	int32 total_duration = 0;

	//levels:[ (levels)
	JsonWriter->WriteArrayStart("levels");
	for (auto& i : ConfigData)
	{

		int32 size_x = FMath::RoundToInt32(i.MapSizeInCentimeter.X) == 0 ? i.MapImage->GetSizeX() : FMath::RoundToInt32(i.MapSizeInCentimeter.X);
		int32 size_y = FMath::RoundToInt32(i.MapSizeInCentimeter.Y) == 0 ? i.MapImage->GetSizeY() : FMath::RoundToInt32(i.MapSizeInCentimeter.Y);
		//{ (level)
		JsonWriter->WriteObjectStart();

		JsonWriter->WriteValue("name", i.name);
		JsonWriter->WriteValue("path", FPaths::GetBaseFilename(i.Map.ToSoftObjectPath().ToString(), false));
		JsonWriter->WriteValue("size_x", size_x);
		JsonWriter->WriteValue("size_y", size_y);
		JsonWriter->WriteValue("start_x", 0);
		JsonWriter->WriteValue("start_y", 0);
		JsonWriter->WriteValue("start_z", 0);
		JsonWriter->WriteValue("start_rot", 0.f);
		JsonWriter->WriteValue("loc_x", 0);
		JsonWriter->WriteValue("loc_y", 0);
		JsonWriter->WriteValue("loc_z", 0);
		JsonWriter->WriteValue("rot_z", 0.f);
		JsonWriter->WriteValue("in_stage", false);
		JsonWriter->WriteValue("img", Texture2Base64String(i.MapImage));

		//} (level)
		JsonWriter->WriteObjectEnd();
	}
	//] (levels)
	JsonWriter->WriteArrayEnd();

	//t_area_info:[ (t_area_info)
	JsonWriter->WriteArrayStart("t_area_info");
	for (auto& i : TAreaInfo)
	{
		total_duration += i.duration_sec;
		//{ (t_area_info)
		JsonWriter->WriteObjectStart();

		JsonWriter->WriteValue("display_name", i.display_name);
		JsonWriter->WriteValue("index", i.index);
		JsonWriter->WriteValue("duration_sec", i.duration_sec);

		//} (t_area_info)
		JsonWriter->WriteObjectEnd();
	}
	//] (t_area_info)
	JsonWriter->WriteArrayEnd();

	JsonWriter->WriteValue("t_area_num", TAreaInfo.Num());
	JsonWriter->WriteValue("duration_m", total_duration / 60);


	//} (main)
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();

	// 对原始数据文件进行 加密


	// ======================== 仅用于显示用的

	FString JsonViewStr;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonViewWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonViewStr);

	//{ (main)
	JsonViewWriter->WriteObjectStart();

	JsonViewWriter->WriteValue("uid", uuid);
	JsonViewWriter->WriteValue("app_name", app_name);
	JsonViewWriter->WriteValue("content_name", content_name);
	JsonViewWriter->WriteValue("display_name", display_name);
	JsonViewWriter->WriteValue("version", version);
	JsonViewWriter->WriteValue("t_area_num", TAreaInfo.Num());
	JsonViewWriter->WriteValue("duration_m", total_duration / 60);
	// UnrealEngine 获取当前时间，并格式化
	JsonViewWriter->WriteValue("create_time", FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")));
	JsonViewWriter->WriteValue("data", AESEncrypt(JsonStr, GenKey()));

	//} (main)
	JsonViewWriter->WriteObjectEnd();
	JsonViewWriter->Close();

	// ======================== 仅用于显示用的


	//save path
	const FString path = FPaths::ProjectDir() + (app_name + "/") + (version + "/") + content_name + ".info";
	//写入文件
	FFileHelper::SaveStringToFile(JsonViewStr, *path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	if (IsOpen) {
		FString OpenDir = FPaths::ConvertRelativePathToFull(FPaths::GetPath(path));
		FPlatformProcess::ExploreFolder(*OpenDir);
	}
}

FString UPathConfigObject::GetAndroidPackageName()
{
	// 获取Android运行时设置
	UAndroidRuntimeSettings* AndroidSettings = GetMutableDefault<UAndroidRuntimeSettings>();
	if (AndroidSettings)
	{
		UE_LOG(LogTemp, Log, TEXT("GetAndroidPackageName = %s"), *AndroidSettings->PackageName);
		return AndroidSettings->PackageName;
		// 使用PackageName...
	}
	UE_LOG(LogTemp, Error, TEXT("GetAndroidPackageName NONE "));
	return FString();
}



TArray<FKeyFrameData> UPathConfigObject::GetIntegerTrackKeyFrames(ULevelSequence* LevelSequence, FName TrackName, bool& bOutSuccess)
{
	TArray<FKeyFrameData> KeyFrames; 

	if (!LevelSequence)
	{
		bOutSuccess = false;
		return KeyFrames;
	}

	// 获取帧率信息
	int TicksPerFrame = LevelSequence->MovieScene->GetTickResolution().AsDecimal() / LevelSequence->MovieScene->GetDisplayRate().AsDecimal();

	// 遍历所有绑定
	for (const FMovieSceneBinding& Binding : LevelSequence->MovieScene->GetBindings())
	{
		// 查找指定名称的整数轨道
		UMovieSceneIntegerTrack* IntegerTrack = Cast<UMovieSceneIntegerTrack>(LevelSequence->MovieScene->FindTrack(UMovieSceneIntegerTrack::StaticClass(), Binding.GetObjectGuid(), TrackName));
		if (IntegerTrack)
		{
			// 获取所有Section
			for (UMovieSceneSection* Section : IntegerTrack->GetAllSections())
			{
				if (UMovieSceneIntegerSection* IntegerSection = Cast<UMovieSceneIntegerSection>(Section))
				{
					// 获取Integer通道
					FMovieSceneIntegerChannel* Channel = IntegerSection->GetChannelProxy().GetChannel<FMovieSceneIntegerChannel>(0);
					if (Channel)
					{
						// 获取所有关键帧
						TArray<FFrameNumber> KeyTimes;
						TArray<FKeyHandle> KeyHandles;
						Channel->GetKeys(TRange<FFrameNumber>::All(), &KeyTimes, &KeyHandles);

						// 遍历所有关键帧
						for (int32 i = 0; i < KeyTimes.Num(); ++i)
						{
							FKeyFrameData KeyFrame;
							// 将tick转换为帧数
							KeyFrame.Frame = KeyTimes[i].Value / TicksPerFrame;
							// 获取关键帧值
							KeyFrame.Value = Channel->GetData().GetValues()[i];
							KeyFrames.Add(KeyFrame);
						}
					}
				}
			}
			break; // 找到轨道后退出循环
		}
	}

	if (KeyFrames.Num() > 0)
	{
		bOutSuccess = true;
	}
	else
	{
		bOutSuccess = false;
	}

	return KeyFrames;
}

// 获取子序列对象及其路径的函数
bool UPathConfigObject::GetSubsequenceByName(ULevelSequence* LevelSequence, FString SubSequenceName, ULevelSequence*& OutSubSequence, FString& OutPackagePath)
{
	if (!LevelSequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LevelSequence provided"));
		return false;
	}
	// 获取序列中的所有 subsequence tracks
	TArray<FString> SubSequenceNames;

	// 遍历所有轨道
	for (UMovieSceneTrack* Track : LevelSequence->MovieScene->GetTracks())
	{
		// 检查是否是子序列轨道
		if (UMovieSceneSubTrack* SubTrack = Cast<UMovieSceneSubTrack>(Track))
		{
			// 遍历子序列轨道中的所有Section
			for (UMovieSceneSection* Section : SubTrack->GetAllSections())
			{
				if (UMovieSceneSubSection* SubSection = Cast<UMovieSceneSubSection>(Section))
				{
					// 获取子序列
					UMovieSceneSequence* SubSequence = SubSection->GetSequence();
					if (SubSequence)
					{
						// 获取子序列的名称
						FString name = SubSequence->GetName();
						if (name.Contains(SubSequenceName))
						{
							OutPackagePath = SubSequence->GetOuter()->GetPathName();
							OutSubSequence = Cast<ULevelSequence>(SubSequence);
							SubSequenceNames.Add(SubSequenceName);
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

#if WITH_EDITOR
#pragma optimize("", on)
#endif


