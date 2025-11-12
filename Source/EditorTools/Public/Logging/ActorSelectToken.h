// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"
#include "GameFramework/Actor.h"

/**
 * 自定义消息日志 Token，用于选择 Actor 而不移动摄像机
 */
class EDITORTOOLS_API FActorSelectToken : public IMessageToken
{
public:
	/**
	 * 创建一个可点击的 Actor 选择 Token
	 * @param Actor 要选择的 Actor
	 * @param DisplayText 显示的文本
	 */
	static TSharedRef<FActorSelectToken> Create(AActor* Actor, const FText& DisplayText);

	// IMessageToken interface
	virtual EMessageToken::Type GetType() const override { return EMessageToken::Text; }

private:
	FActorSelectToken(AActor* InActor, const FText& InDisplayText);

	TWeakObjectPtr<AActor> Actor;
};

