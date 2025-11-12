// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

// 定义编辑器工具插件的日志类别
DECLARE_LOG_CATEGORY_EXTERN(LogEditorTools, Log, All);

// 日志宏定义，用于不同颜色的输出
#define UE_LOG_EDITORTOOLS_INFO(Format, ...)		UE_LOG(LogEditorTools, Log, Format, ##__VA_ARGS__)
#define UE_LOG_EDITORTOOLS_WARNING(Format, ...)	UE_LOG(LogEditorTools, Warning, Format, ##__VA_ARGS__)
#define UE_LOG_EDITORTOOLS_ERROR(Format, ...)		UE_LOG(LogEditorTools, Error, Format, ##__VA_ARGS__)
#define UE_LOG_EDITORTOOLS_DISPLAY(Format, ...)		UE_LOG(LogEditorTools, Display, Format, ##__VA_ARGS__)

// 未使用资源检查专用日志宏（使用不同颜色）
#define UE_LOG_UNUSED_ASSETS_HEADER(Format, ...)	UE_LOG(LogEditorTools, VeryVerbose, Format, ##__VA_ARGS__)  // 绿色 - 标题
#define UE_LOG_UNUSED_ASSETS_INFO(Format, ...)		UE_LOG(LogEditorTools, Log, Format, ##__VA_ARGS__)         // 白色 - 信息
#define UE_LOG_UNUSED_ASSETS_LIST(Format, ...)		UE_LOG(LogEditorTools, Warning, Format, ##__VA_ARGS__)     // 黄色 - 列表项
#define UE_LOG_UNUSED_ASSETS_STATS(Format, ...)		UE_LOG(LogEditorTools, VeryVerbose, Format, ##__VA_ARGS__)  // 绿色 - 统计信息

