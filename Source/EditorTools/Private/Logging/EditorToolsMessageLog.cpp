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
#include "Logging/AssetObjectToken.h"
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
	const TArray<FString>& FolderPaths,
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
	FString FolderPathsText;
	if (FolderPaths.Num() == 1)
	{
		FolderPathsText = FolderPaths[0];
	}
	else
	{
		FolderPathsText = FString::Printf(TEXT("%d个文件夹"), FolderPaths.Num());
	}

	MessageLogListing->AddMessage(
		FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::Format(LOCTEXT("FolderPath", "文件夹路径: {0}"), FText::FromString(FolderPathsText))
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
		// 计算对齐信息
		const int32 RankWidth = FString::FromInt(UnusedAssets.Num()).Len();
		int32 MaxNameLen = 0;
		for (const FUnusedAssetInfo& InfoForWidth : UnusedAssets)
		{
			MaxNameLen = FMath::Max(MaxNameLen, InfoForWidth.AssetName.Len());
		}

		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				LOCTEXT("UnusedAssetsListHeader", "详细资产列表（点击名称可在内容浏览器中定位）：")
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
			
			// 创建可点击的消息（格式：序号 + 类型）
			FString RankStr = FString::FromInt(i + 1);
			// 单个数字时 # 和数字之间有空格，多个数字时没有空格
			// 对于单个数字，先添加空格，然后进行左填充；对于多个数字，直接左填充
			if ((i + 1) < 10)
			{
				RankStr = FString::Printf(TEXT(" %s"), *RankStr);
			}
			RankStr = RankStr.LeftPad(RankWidth);
			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
				EMessageSeverity::Warning,
				FText::FromString(FString::Printf(TEXT("#%s. [%s] "), *RankStr, *AssetType))
			);

			// 添加可点击的资产链接（名称左对齐，右填充到最大宽度）
			FString PaddedName = Info.AssetName;
			if (PaddedName.Len() < MaxNameLen)
			{
				PaddedName += FString::ChrN(MaxNameLen - PaddedName.Len(), TEXT(' '));
			}
			if (AssetObject)
			{
				// 手动添加放大镜图标以保持与 GetHighPolyActorsInScene 的一致性
				// 使用自定义的 FAssetObjectToken，不自动显示放大镜图标
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
				Message->AddToken(FAssetObjectToken::Create(AssetObject, FText::FromString(PaddedName)));
			}
			else
			{
				// 如果无法加载资产，使用文本显示，需要手动添加放大镜图标
				Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
				Message->AddToken(FTextToken::Create(FText::FromString(PaddedName)));
			}
			
			// 添加路径信息
			Message->AddToken(FTextToken::Create(FText::Format(LOCTEXT("AssetPath", " ({0})"), FText::FromString(FullAssetPath))));

			MessageLogListing->AddMessage(Message);
		}
	}
	
	// 计算分隔线长度（与列表对齐）
	int32 SeparatorLen = 80;
	FString FooterSeparator;
	if (UnusedAssets.Num() > 0)
	{
		// 如果有未使用的资产，根据列表宽度计算分隔线长度
		const int32 RankWidth = FString::FromInt(UnusedAssets.Num()).Len();
		int32 MaxNameLen = 0;
		for (const FUnusedAssetInfo& InfoForWidth : UnusedAssets)
		{
			MaxNameLen = FMath::Max(MaxNameLen, InfoForWidth.AssetName.Len());
		}
		SeparatorLen = FMath::Clamp(RankWidth + MaxNameLen + 50, 60, 120);
	}
	FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
	
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

