// Copyright 2021 Justin Kiesskalt, All Rights Reserved.
#
#pragma once
#
#include "CoreMinimal.h"
#
#if WITH_EDITOR
class IMessageLogListing;
#endif

/**
 * Editor-only utility helpers for common Content Browser interactions, etc.
 */
class UEditorToolsUtilities
{
public:
	/**
	 * Resolve a usable folder path based on an optional input and the current Content Browser selection.
	 *
	 * - If InputFolderPath is non-empty, it's copied to OutFolderPath and returns true.
	 * - If InputFolderPath is empty, tries to fetch the first selected path from the Content Browser.
	 * - If nothing is selected, shows a dialog with NoSelectionDialogText and returns false.
	 *
	 * Returns true if a path was resolved into OutFolderPath, false otherwise.
	 *
	 * Note: Caller is responsible for normalizing the returned path if needed.
	 */
	static bool ResolveContentBrowserFolder(const FString& InputFolderPath, FString& OutFolderPath, const FText& NoSelectionDialogText);
#if WITH_EDITOR
	/**
	 * Retrieve the default EditorTools message log listing, optionally clearing previous messages.
	 */
	static TSharedPtr<IMessageLogListing> GetOrCreateMessageLogListing(bool bClearExistingMessages);

	/**
	 * Write a warning message to the default EditorTools Message Log and open the log panel.
	 */
	static void LogWarningToMessageLogAndOpen(const FText& Message);

	/**
	 * Add a simple info-severity entry to the EditorTools message log if the listing is valid.
	 */
	static void AddInfoMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, const FText& MessageText);

	/**
	 * Add a simple warning-severity entry to the EditorTools message log if the listing is valid.
	 */
	static void AddWarningMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, const FText& MessageText);

	/**
	 * Add a simple error-severity entry to the EditorTools message log if the listing is valid.
	 */
	static void AddErrorMessage(const TSharedPtr<IMessageLogListing>& MessageLogListing, const FText& MessageText);

	/**
	 * Open the default EditorTools Message Log panel without adding a message.
	 */
	static void OpenMessageLogPanel();
#endif
};


