// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "Logging/ActorSelectToken.h"
#include "Editor.h"

TSharedRef<FActorSelectToken> FActorSelectToken::Create(AActor* Actor, const FText& DisplayText)
{
	return MakeShareable(new FActorSelectToken(Actor, DisplayText));
}

FActorSelectToken::FActorSelectToken(AActor* InActor, const FText& InDisplayText)
	: Actor(InActor)
{
	CachedText = InDisplayText;

	// 设置点击回调，只选择 Actor，不移动摄像机
	MessageTokenActivated = FOnMessageTokenActivated::CreateLambda([WeakActor = Actor](const TSharedRef<IMessageToken>&)
	{
#if WITH_EDITOR
		if (GEditor)
		{
			if (AActor* ActorPtr = WeakActor.Get())
			{
				// 强制单选：先清空选择，再选中指定Actor；不移动摄像机
				GEditor->SelectNone(/*bNoteSelectionChange*/true, /*bDeselectBSPSurfs*/true, /*WarnAboutManyActors*/false);
				GEditor->SelectActor(ActorPtr, /*bInSelected*/true, /*bNotify*/true);
			}
		}
#endif
	});
}

