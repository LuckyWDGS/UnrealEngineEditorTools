// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FEditorToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** 注册工具栏菜单扩展 */
	void RegisterMenuExtensions();
	
	/** 打开编辑器工具窗口 */
	void OpenEditorToolWidget();
	
	/** 工具栏按钮点击回调 */
	void OnToolbarButtonClicked();
};

DECLARE_LOG_CATEGORY_EXTERN(EditorToolsLog, Log, All);