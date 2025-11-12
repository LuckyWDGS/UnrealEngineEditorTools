#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Utility helpers for presenting collision-related descriptions in UI.
 */
struct FEditorToolsCollisionText
{
	static FORCEINLINE FText GetCollisionEnabledDescription(ECollisionEnabled::Type CollisionMode)
	{
		switch (CollisionMode)
		{
		case ECollisionEnabled::NoCollision:
			return NSLOCTEXT("EditorToolsCollision", "CollisionMode_NoCollision", "无碰撞");
		case ECollisionEnabled::QueryOnly:
			return NSLOCTEXT("EditorToolsCollision", "CollisionMode_QueryOnly", "仅查询");
		case ECollisionEnabled::PhysicsOnly:
			return NSLOCTEXT("EditorToolsCollision", "CollisionMode_PhysicsOnly", "仅物理");
		case ECollisionEnabled::QueryAndPhysics:
			return NSLOCTEXT("EditorToolsCollision", "CollisionMode_QueryAndPhysics", "查询与物理");
		default:
			break;
		}

		return NSLOCTEXT("EditorToolsCollision", "CollisionMode_Unknown", "未知");
	}
};

