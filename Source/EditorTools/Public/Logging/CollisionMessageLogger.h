// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"

#if WITH_EDITOR
class IMessageLogListing;

/**
 * 记录静态网格体Actor的碰撞关闭信息
 */
struct FDisableCollisionActorRecord
{
	TWeakObjectPtr<AStaticMeshActor> Actor;
	FString ActorLabel;
	FName PreviousProfile = NAME_None;
	bool bGeneratedOverlap = false;
};

/**
 * 碰撞消息日志记录器
 * 用于将碰撞关闭相关的消息记录到消息日志中，并提供可点击的Actor选择功能
 */
class EDITORTOOLS_API FCollisionMessageLogger
{
public:
	/**
	 * 记录静态网格体碰撞关闭的消息日志
	 * @param MessageLogListing 消息日志列表
	 * @param Records 碰撞关闭记录数组
	 * @param TotalSelectedActors 总选择的Actor数量
	 * @param bIncludeHeaderAndFooter 是否包含头部和尾部信息
	 */
	static void LogDisableCollisionMessages(
		TSharedPtr<IMessageLogListing> MessageLogListing,
		const TArray<FDisableCollisionActorRecord>& Records,
		int32 TotalSelectedActors,
		bool bIncludeHeaderAndFooter
	);
};
#endif

