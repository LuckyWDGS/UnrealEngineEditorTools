// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "Logging/EditorToolsMessageLog.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "MessageLogInitializationOptions.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Logging/EditorToolsLog.h"
#include "Misc/UObjectToken.h"
#include "Logging/TokenizedMessage.h"
#include "UObject/SoftObjectPath.h"
#include "GameFramework/Actor.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "LevelEditorViewport.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "FEditorToolsMessageLog"

const FName FEditorToolsMessageLog::MessageLogName = TEXT("EditorTools");

void FEditorToolsMessageLog::Initialize()
{
#if WITH_EDITOR
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowPages = true;
	InitOptions.bShowFilters = true;
	InitOptions.bAllowClear = true;
	MessageLogModule.RegisterLogListing(MessageLogName, LOCTEXT("EditorToolsMessageLog", "编辑器工具"), InitOptions);
#endif
}

void FEditorToolsMessageLog::ShowUnusedAssetsReport(
	const FString& AssetType,
	const FString& FolderPath,
	const TArray<FUnusedAssetInfo>& UnusedAssets,
	int32 TotalCount)
{
#if WITH_EDITOR
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedPtr<IMessageLogListing> MessageLogListing = MessageLogModule.GetLogListing(MessageLogName);
	
	if (!MessageLogListing.IsValid())
	{
		Initialize();
		MessageLogListing = MessageLogModule.GetLogListing(MessageLogName);
	}

	if (!MessageLogListing.IsValid())
	{
		UE_LOG_EDITORTOOLS_ERROR(TEXT("无法创建消息日志"));
		return;
	}

	// 清空之前的消息
	MessageLogListing->ClearMessages();

	// 添加标题
	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::Format(LOCTEXT("UnusedAssetsHeader", "------------------ 检查未使用的{0} ------------------"), FText::FromString(AssetType))
		)
	);

	// 添加文件夹路径信息
	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::Format(LOCTEXT("FolderPath", "文件夹路径: {0}"), FText::FromString(FolderPath))
		)
	);

	// 添加统计信息
	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::Format(LOCTEXT("UnusedAssetsStats", "找到 {0} 个{1}资源，发现 {2} 个未使用的{1}"), 
				FText::AsNumber(TotalCount),
				FText::FromString(AssetType),
				FText::AsNumber(UnusedAssets.Num()))
		)
	);

	// 添加未使用资产列表
	if (UnusedAssets.Num() > 0)
	{
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				LOCTEXT("UnusedAssetsListHeader", "未使用的资产列表:")
			)
		);

		for (int32 i = 0; i < UnusedAssets.Num(); ++i)
		{
			const FUnusedAssetInfo& Info = UnusedAssets[i];
			FString FullAssetPath = FString::Printf(TEXT("%s/%s"), *Info.AssetPath, *Info.AssetName);
			
			// 构建资产对象路径（格式：/Game/Path/AssetName.AssetName）
			FString ObjectPath = FullAssetPath;
			if (!ObjectPath.StartsWith(TEXT("/Game/")))
			{
				if (ObjectPath.StartsWith(TEXT("/")))
				{
					ObjectPath = TEXT("/Game") + ObjectPath;
				}
				else
				{
					ObjectPath = TEXT("/Game/") + ObjectPath;
				}
			}
			ObjectPath = ObjectPath + TEXT(".") + Info.AssetName;
			
			// 尝试加载资产对象
			UObject* AssetObject = LoadObject<UObject>(nullptr, *ObjectPath);
			
			// 创建可点击的消息
			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				FText::Format(LOCTEXT("UnusedAssetItem", "  {0}. "), i + 1)
			);

			// 添加可点击的资产链接（使用 UObjectToken）
			if (AssetObject)
			{
				Message->AddToken(FUObjectToken::Create(AssetObject, FText::FromString(Info.AssetName)));
			}
			else
			{
				// 如果无法加载资产，使用文本显示
				Message->AddToken(FTextToken::Create(FText::FromString(Info.AssetName)));
			}
			Message->AddToken(FTextToken::Create(FText::Format(LOCTEXT("AssetPath", " ({0})"), FText::FromString(FullAssetPath))));

			MessageLogListing->AddMessage(Message);
		}
	}
	int32 SeparatorLen = 80;
	FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
	// 添加结束分隔线
	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::FromString(FooterSeparator)
		)
	);

	// 打开消息日志窗口
	MessageLogModule.OpenMessageLog(MessageLogName);
#endif
}

void FEditorToolsMessageLog::NavigateToAsset(const FString& AssetPath, const FString& AssetName)
{
#if WITH_EDITOR
	// 构建完整的资产路径
	FString FullAssetPath = FString::Printf(TEXT("%s/%s"), *AssetPath, *AssetName);
	
	// 移除开头的 /Game/ 如果存在
	FString CleanPath = FullAssetPath;
	if (CleanPath.StartsWith(TEXT("/Game/")))
	{
		CleanPath = CleanPath.Mid(6); // 移除 "/Game/"
	}
	else if (CleanPath.StartsWith(TEXT("/")))
	{
		CleanPath = CleanPath.Mid(1); // 移除开头的 "/"
	}

	// 构建资产对象路径（格式：/Game/Path/AssetName.AssetName）
	FString ObjectPath = FString::Printf(TEXT("/Game/%s.%s"), *CleanPath, *AssetName);

	// 尝试加载资产
	UObject* Asset = LoadObject<UObject>(nullptr, *ObjectPath);
	
	if (Asset)
	{
		// 使用内容浏览器定位到资产
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<UObject*> AssetsToSync;
		AssetsToSync.Add(Asset);
		ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);
	}
		else
		{
			// 如果直接加载失败，尝试通过资产注册表查找
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
			
			FSoftObjectPath SoftObjectPath(ObjectPath);
			FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(SoftObjectPath);
			
			if (AssetData.IsValid())
			{
				// 使用资产数据定位
				FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				TArray<FAssetData> AssetsToSync;
				AssetsToSync.Add(AssetData);
				ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);
			}
			else
			{
				UE_LOG_EDITORTOOLS_WARNING(TEXT("无法找到资产: %s"), *ObjectPath);
			}
		}
#endif
}

void FEditorToolsMessageLog::NavigateToActor(AActor* Actor)
{
#if WITH_EDITOR
	if (!Actor)
	{
		return;
	}

	// 选择Actor
	GEditor->SelectActor(Actor, true, true);
	
	// 聚焦到Actor
	GEditor->MoveViewportCamerasToActor(*Actor, false);
#endif
}

#undef LOCTEXT_NAMESPACE

