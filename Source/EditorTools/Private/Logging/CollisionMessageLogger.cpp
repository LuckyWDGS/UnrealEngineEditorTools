// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "Logging/CollisionMessageLogger.h"
#include "IMessageLogListing.h"
#include "MessageLogModule.h"
#include "Logging/ActorSelectToken.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/DisplayNameUtils.h"
#include "EditorToolsUtilities.h"

#define LOCTEXT_NAMESPACE "FCollisionMessageLogger"

#if WITH_EDITOR
void FCollisionMessageLogger::LogDisableCollisionMessages(
	TSharedPtr<IMessageLogListing> MessageLogListing,
	const TArray<FDisableCollisionActorRecord>& Records,
	int32 TotalSelectedActors,
	bool bIncludeHeaderAndFooter)
{
	if (!MessageLogListing.IsValid() || Records.Num() == 0)
	{
		return;
	}

	if (bIncludeHeaderAndFooter)
	{
		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			LOCTEXT("DisableCollisionHeader", "------------------ 静态网格体碰撞关闭 ------------------")
		);

		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::Format(
				LOCTEXT("DisableCollisionSummary", "选择了 {0} 个 Actor，其中 {1} 个静态网格体碰撞已切换为【无碰撞】："),
				FText::AsNumber(TotalSelectedActors),
				FText::AsNumber(Records.Num())
			)
		);
	}

	const int32 RankWidth = FString::FromInt(Records.Num()).Len();

	int32 Index = 1;
	for (const FDisableCollisionActorRecord& Record : Records)
	{
		AStaticMeshActor* StaticMeshActor = Record.Actor.Get();
		if (!IsValid(StaticMeshActor))
		{
			continue;
		}

		FString RankStr = FString::FromInt(Index++);
		if (RankStr.Len() == 1)
		{
			RankStr = FString::Printf(TEXT(" %s"), *RankStr);
		}
		RankStr = RankStr.LeftPad(RankWidth);

		const FString DisplayName = EditorTools::BuildFixedDisplayName(Record.ActorLabel);
		const FText DisplayText = FText::FromString(DisplayName);

		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
			EMessageSeverity::Info,
			FText::FromString(FString::Printf(TEXT("#%s. "), *RankStr))
		);

		Message->AddToken(FImageToken::Create(TEXT("Icons.Search")));
		Message->AddToken(FActorSelectToken::Create(StaticMeshActor, DisplayText));

		if (Record.PreviousProfile != NAME_None)
		{
			Message->AddToken(
				FTextToken::Create(
					FText::Format(
						LOCTEXT("DisableCollisionProfileToken", "[原碰撞预设: {0}]"),
						FText::FromName(Record.PreviousProfile)
					)
				)
			);
		}

		if (Record.bGeneratedOverlap)
		{
			Message->AddToken(FTextToken::Create(LOCTEXT("DisableCollisionOverlapToken", "[原重叠事件: 开启]")));
		}

		Message->AddToken(FTextToken::Create(LOCTEXT("DisableCollisionResultToken", ">> 现已设置为【无碰撞】，并关闭重叠和碰撞通知")));

		MessageLogListing->AddMessage(Message);
	}

	if (bIncludeHeaderAndFooter && Records.Num() > 0)
	{
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				LOCTEXT("DisableCollisionTips", "提示：可以在“世界概览”中使用“选择”按钮快速定位，并根据需要手动恢复原碰撞。")
			)
		);

		const int32 SeparatorLen = 80;
		const FString FooterSeparator = FString::ChrN(SeparatorLen, TEXT('-'));
		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::FromString(FooterSeparator)
		);
	}
}
#endif

#undef LOCTEXT_NAMESPACE

