// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Types/UnusedAssetTypes.h"

/**
 * 编辑器工具消息日志辅助类
 * 用于创建可点击的日志消息，支持定位到内容浏览器中的资产
 */
class EDITORTOOLS_API FEditorToolsMessageLog
{
public:
	/**
	 * 初始化消息日志系统
	 */
	static void Initialize();

	/**
	 * 显示未使用资产的检查结果
	 * @param AssetType 资产类型（如"模型"、"材质"、"贴图"）
	 * @param FolderPath 检查的文件夹路径
	 * @param UnusedAssets 未使用的资产列表
	 * @param TotalCount 总资产数量
	 */
	static void ShowUnusedAssetsReport(
		const FString& AssetType,
		const FString& FolderPath,
		const TArray<FUnusedAssetInfo>& UnusedAssets,
		int32 TotalCount
	);

	/**
	 * 在内容浏览器中定位到指定资产
	 * @param AssetPath 资产路径（如 "/Game/Models/MyMesh"）
	 * @param AssetName 资产名称（如 "MyMesh"）
	 */
	static void NavigateToAsset(const FString& AssetPath, const FString& AssetName);

	/**
	 * 在场景中定位到指定Actor（选择和聚焦）
	 * @param Actor 要定位的Actor
	 */
	static void NavigateToActor(AActor* Actor);

	/** 消息日志名称 */
	static const FName MessageLogName;
};

