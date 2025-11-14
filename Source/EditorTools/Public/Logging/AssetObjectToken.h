// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"
#include "UObject/UObjectGlobals.h"

/**
 * 自定义消息日志 Token，用于在内容浏览器中定位资产，不自动显示放大镜图标
 */
class EDITORTOOLS_API FAssetObjectToken : public IMessageToken
{
public:
	/**
	 * 创建一个可点击的资产对象 Token
	 * @param Object 要定位的 UObject
	 * @param DisplayText 显示的文本
	 */
	static TSharedRef<FAssetObjectToken> Create(UObject* Object, const FText& DisplayText);

	// IMessageToken interface
	virtual EMessageToken::Type GetType() const override { return EMessageToken::Text; }

private:
	FAssetObjectToken(UObject* InObject, const FText& InDisplayText);

	TWeakObjectPtr<UObject> Object;
};

