// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * 编辑器工具样式管理类
 * 负责加载和管理自定义图标和样式
 */
class FEditorToolsStyle
{
public:
	/** 初始化样式集 */
	static void Initialize();

	/** 关闭并清理样式集 */
	static void Shutdown();

	/** 获取样式集实例 */
	static TSharedPtr<class ISlateStyle> Get();

	/** 获取样式集名称 */
	static FName GetStyleSetName();

private:
	/** 创建样式集 */
	static TSharedRef<class FSlateStyleSet> Create();

private:
	/** 样式集实例 */
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};

