#pragma once

#include "CoreMinimal.h"

namespace EditorTools
{
	inline FString BuildFixedDisplayName(const FString& SourceName)
	{
		FString Display = SourceName;
		if (Display.Len() > 8)
		{
			Display = Display.Left(8) + TEXT("...");
		}
		if (Display.Len() < 11)
		{
			Display += FString::ChrN(11 - Display.Len(), TEXT(' '));
		}
		return Display;
	}
}

