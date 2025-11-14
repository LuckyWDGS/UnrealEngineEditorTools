// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "Logging/AssetObjectToken.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"

TSharedRef<FAssetObjectToken> FAssetObjectToken::Create(UObject* Object, const FText& DisplayText)
{
	return MakeShareable(new FAssetObjectToken(Object, DisplayText));
}

FAssetObjectToken::FAssetObjectToken(UObject* InObject, const FText& InDisplayText)
	: Object(InObject)
{
	CachedText = InDisplayText;

	// 设置点击回调，在内容浏览器中定位资产
	MessageTokenActivated = FOnMessageTokenActivated::CreateLambda([WeakObject = Object](const TSharedRef<IMessageToken>&)
	{
#if WITH_EDITOR
		if (UObject* ObjectPtr = WeakObject.Get())
		{
			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();
			
			// 在内容浏览器中定位并选中资产
			TArray<FAssetData> AssetsToSelect;
			AssetsToSelect.Add(FAssetData(ObjectPtr));
			ContentBrowser.SyncBrowserToAssets(AssetsToSelect);
		}
#endif
	});
}

