// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"

#if WITH_EDITOR
class IMessageLogListing;

/**
 * 记录静态网格体Actor的阴影关闭信息
 */
struct FDisableShadowActorRecord
{
	TWeakObjectPtr<AStaticMeshActor> Actor;
	FString ActorLabel;
	bool bPreviousCastShadow = false;
};

/**
 * 阴影消息日志记录器
 * 用于将阴影关闭相关的消息记录到消息日志中，并提供可点击的Actor选择功能
 */
class EDITORTOOLS_API FShadowMessageLogger
{
public:
	/**
	 * 记录静态网格体阴影关闭的消息日志
	 * @param MessageLogListing 消息日志列表
	 * @param Records 阴影关闭记录数组
	 * @param TotalSelectedActors 总选择的Actor数量
	 * @param bIncludeHeaderAndFooter 是否包含头部和尾部信息
	 */
	static void LogDisableShadowMessages(
		TSharedPtr<IMessageLogListing> MessageLogListing,
		const TArray<FDisableShadowActorRecord>& Records,
		int32 TotalSelectedActors,
		bool bIncludeHeaderAndFooter
	);
};
#endif

