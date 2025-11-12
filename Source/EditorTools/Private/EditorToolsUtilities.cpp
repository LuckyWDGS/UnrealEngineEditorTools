// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#
#include "EditorToolsUtilities.h"
#
#if WITH_EDITOR
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/EditorToolsMessageLog.h"
#endif
#
bool UEditorToolsUtilities::ResolveContentBrowserFolder(const FString& InputFolderPath, FString& OutFolderPath, const FText& NoSelectionDialogText)
{
#if WITH_EDITOR
	// If input is provided, use it.
	if (!InputFolderPath.IsEmpty())
	{
		OutFolderPath = InputFolderPath;
		return true;
	}
	// Otherwise try to read Content Browser selection.
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FString> SelectedFolders;
	ContentBrowserModule.Get().GetSelectedFolders(SelectedFolders);
	if (SelectedFolders.Num() > 0)
	{
		OutFolderPath = SelectedFolders[0];
		return true;
	}
	// No selection; just return false. Caller can decide how to notify (e.g., message log).
	return false;
#else
	OutFolderPath = InputFolderPath;
	return !OutFolderPath.IsEmpty();
#endif
}

#if WITH_EDITOR
namespace
{
	static void AddSimpleMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, EMessageSeverity::Type Severity, const FText& MessageText)
	{
		if (MessageLogListing.IsValid())
		{
			MessageLogListing->AddMessage(FTokenizedMessage::Create(Severity, MessageText));
		}
	}
}

TSharedPtr<IMessageLogListing> UEditorToolsUtilities::GetOrCreateMessageLogListing(bool bClearExistingMessages)
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedPtr<IMessageLogListing> MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);

	if (!MessageLogListing.IsValid())
	{
		FEditorToolsMessageLog::Initialize();
		MessageLogListing = MessageLogModule.GetLogListing(FEditorToolsMessageLog::MessageLogName);
	}

	if (MessageLogListing.IsValid() && bClearExistingMessages)
	{
		MessageLogListing->ClearMessages();
	}

	return MessageLogListing;
}

void UEditorToolsUtilities::LogWarningToMessageLogAndOpen(const FText& Message)
{
	AddWarningMessage(GetOrCreateMessageLogListing(false), Message);
	OpenMessageLogPanel();
}

void UEditorToolsUtilities::AddInfoMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, const FText& MessageText)
{
	AddSimpleMessage(MessageLogListing, EMessageSeverity::Info, MessageText);
}

void UEditorToolsUtilities::AddWarningMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, const FText& MessageText)
{
	AddSimpleMessage(MessageLogListing, EMessageSeverity::Warning, MessageText);
}

void UEditorToolsUtilities::AddErrorMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, const FText& MessageText)
{
	AddSimpleMessage(MessageLogListing, EMessageSeverity::Error, MessageText);
}
#endif

#if WITH_EDITOR
void UEditorToolsUtilities::OpenMessageLogPanel()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.OpenMessageLog(FEditorToolsMessageLog::MessageLogName);
}
#endif


