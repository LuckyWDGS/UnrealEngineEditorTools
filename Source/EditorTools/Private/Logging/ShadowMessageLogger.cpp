// Copyright 2021 Justin Kiesskalt, All Rights Reserved.

#include "Logging/ShadowMessageLogger.h"
#include "IMessageLogListing.h"
#include "MessageLogModule.h"
#include "Logging/ActorSelectToken.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/DisplayNameUtils.h"
#include "EditorToolsUtilities.h"

#define LOCTEXT_NAMESPACE "FShadowMessageLogger"

#if WITH_EDITOR
void FShadowMessageLogger::LogDisableShadowMessages(
	TSharedPtr<IMessageLogListing> MessageLogListing,
	const TArray<FDisableShadowActorRecord>& Records,
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
			LOCTEXT("DisableShadowHeader", "------------------ 静态网格体阴影关闭 ------------------")
		);

		UEditorToolsUtilities::AddInfoMessage(
			MessageLogListing,
			FText::Format(
				LOCTEXT("DisableShadowSummary", "选择了 {0} 个 Actor，其中 {1} 个静态网格体阴影已关闭："),
				FText::AsNumber(TotalSelectedActors),
				FText::AsNumber(Records.Num())
			)
		);
	}

	const int32 RankWidth = FString::FromInt(Records.Num()).Len();

	int32 Index = 1;
	for (const FDisableShadowActorRecord& Record : Records)
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

		if (Record.bPreviousCastShadow)
		{
			Message->AddToken(FTextToken::Create(LOCTEXT("DisableShadowPreviousToken", "[原投射阴影: 开启]")));
		}

		Message->AddToken(FTextToken::Create(LOCTEXT("DisableShadowResultToken", ">> 现已关闭投射阴影")));

		MessageLogListing->AddMessage(Message);
	}

	if (bIncludeHeaderAndFooter && Records.Num() > 0)
	{
		MessageLogListing->AddMessage(
			FTokenizedMessage::Create(
				EMessageSeverity::Info,
				LOCTEXT("DisableShadowTips", "提示：可以在“世界概览”中使用“选择”按钮快速定位，并根据需要手动恢复投射阴影。")
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

