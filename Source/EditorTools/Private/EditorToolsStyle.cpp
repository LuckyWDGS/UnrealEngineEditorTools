// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "EditorToolsStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"

TSharedPtr<FSlateStyleSet> FEditorToolsStyle::StyleInstance = nullptr;

void FEditorToolsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FEditorToolsStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		StyleInstance.Reset();
	}
}

TSharedRef<FSlateStyleSet> FEditorToolsStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	// 获取插件根目录
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("EditorTools"));
	if (Plugin.IsValid())
	{
		FString ContentDir = Plugin->GetBaseDir();
		Style->SetContentRoot(ContentDir);

		// 注册自定义工具栏图标
		FString IconPath = ContentDir / TEXT("Resources/Ico/Tools.png");
		Style->Set("EditorTools.ToolbarIcon", new FSlateImageBrush(IconPath, FVector2D(40.0f, 40.0f)));

		UE_LOG(LogTemp, Log, TEXT("EditorTools 自定义样式已创建，图标路径: %s"), *IconPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("无法找到 EditorTools 插件，样式创建失败"));
	}

	return Style;
}

TSharedPtr<ISlateStyle> FEditorToolsStyle::Get()
{
	return StyleInstance;
}

FName FEditorToolsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("EditorToolsStyle"));
	return StyleSetName;
}

