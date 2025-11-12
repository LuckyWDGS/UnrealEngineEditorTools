// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "EditorTools.h"
#include "EditorToolsStyle.h"
#include "Logging/EditorToolsMessageLog.h"

#include "Interfaces/IPluginManager.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FEditorToolsModule"
IMPLEMENT_MODULE(FEditorToolsModule, EditorTools)
DEFINE_LOG_CATEGORY(EditorToolsLog);

void FEditorToolsModule::StartupModule()
{
	// 初始化自定义样式
	FEditorToolsStyle::Initialize();
	
	// 初始化消息日志系统
	FEditorToolsMessageLog::Initialize();
	
	// 注册菜单扩展（延迟到 ToolMenus 系统初始化后）
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEditorToolsModule::RegisterMenuExtensions));
}

void FEditorToolsModule::ShutdownModule()
{
	// 清理工具栏扩展
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	
	// 关闭自定义样式
	FEditorToolsStyle::Shutdown();
}

void FEditorToolsModule::RegisterMenuExtensions()
{
	// 获取工具栏菜单
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	if (!ToolbarMenu)
	{
		UE_LOG(EditorToolsLog, Warning, TEXT("无法找到关卡编辑器工具栏菜单"));
		return;
	}

	// 添加工具栏按钮
	FToolMenuSection& Section = ToolbarMenu->AddSection("EditorTools", LOCTEXT("EditorTools", "Editor Tools"));
	
	FToolMenuEntry& Entry = Section.AddEntry(
		FToolMenuEntry::InitToolBarButton(
			"OpenEditorToolWidget",
			FUIAction(FExecuteAction::CreateRaw(this, &FEditorToolsModule::OnToolbarButtonClicked)),
			LOCTEXT("EditorToolWidget", "编辑器工具"),
			LOCTEXT("EditorToolWidgetTooltip", "打开编辑器工具窗口，详细信息在输出日志中查看"),
			FSlateIcon(FEditorToolsStyle::GetStyleSetName(), "EditorTools.ToolbarIcon")  // 使用自定义图标
		)
	);
	
	// 设置按钮的醒目颜色（橙色）
	Entry.StyleNameOverride = "CalloutToolbar";
	
	UE_LOG(EditorToolsLog, Log, TEXT("编辑器工具栏按钮已注册（自定义图标 + 醒目颜色）"));
}

void FEditorToolsModule::OnToolbarButtonClicked()
{
	UE_LOG(EditorToolsLog, Log, TEXT("编辑器工具按钮被点击"));
	OpenEditorToolWidget();
}

void FEditorToolsModule::OpenEditorToolWidget()
{
	// 获取插件的内容目录路径
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("EditorTools"));
	if (!Plugin.IsValid())
	{
		UE_LOG(EditorToolsLog, Error, TEXT("无法找到 EditorTools 插件"));
		
		FNotificationInfo Info(LOCTEXT("PluginNotFound", "无法找到 EditorTools 插件"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return;
	}

	// 构建 Widget 资产路径
	FString WidgetPath = TEXT("/EditorTools/EditorToolWidget.EditorToolWidget");
	
	// 尝试加载 EditorUtilityWidgetBlueprint
	UEditorUtilityWidgetBlueprint* WidgetBlueprint = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, *WidgetPath);
	
	if (!WidgetBlueprint)
	{
		UE_LOG(EditorToolsLog, Warning, TEXT("无法在路径 %s 找到 EditorToolWidget，尝试搜索资产..."), *WidgetPath);
		
		// 如果直接加载失败，尝试通过资产注册表搜索
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		
		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssetsByClass(UEditorUtilityWidgetBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);
		
		for (const FAssetData& AssetData : AssetDataList)
		{
			if (AssetData.AssetName.ToString() == TEXT("EditorToolWidget"))
			{
				WidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(AssetData.GetAsset());
				if (WidgetBlueprint)
				{
					UE_LOG(EditorToolsLog, Log, TEXT("找到 EditorToolWidget: %s"), *AssetData.GetObjectPathString());
					break;
				}
			}
		}
	}

	if (WidgetBlueprint)
	{
		// 使用 EditorUtilitySubsystem 打开 Widget
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		if (EditorUtilitySubsystem)
		{
			EditorUtilitySubsystem->SpawnAndRegisterTab(WidgetBlueprint);
			UE_LOG(EditorToolsLog, Log, TEXT("成功打开编辑器工具窗口"));
		}
		else
		{
			UE_LOG(EditorToolsLog, Error, TEXT("无法获取 EditorUtilitySubsystem"));
		}
	}
	else
	{
		UE_LOG(EditorToolsLog, Error, TEXT("无法找到 EditorToolWidget 资产，请确保已在插件内容目录中创建该资产"));
		
		FNotificationInfo Info(LOCTEXT("WidgetNotFound", "未找到 EditorToolWidget\n请在插件的 Content/EditorTools 目录中创建"));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

#undef LOCTEXT_NAMESPACE
