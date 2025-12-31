#include "ThrowDetectorComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

UThrowDetectorComponent::UThrowDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 预分配数组空间
    PositionHistory.Reserve(POSITION_HISTORY_LENGTH);
    TimeHistory.Reserve(POSITION_HISTORY_LENGTH);
}

void UThrowDetectorComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UThrowDetectorComponent::SetCheckComponent(USceneComponent* component)
{
    this->comp = component;
}



void UThrowDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!comp)
    {
        return;
    }

        
    // 记录当前手部位置和时间
    FVector CurrentPosition = comp->GetComponentLocation();
    float CurrentTime = GetWorld()->GetTimeSeconds();
        
    // 更新位置历史
    PositionHistory.Add(CurrentPosition);
    TimeHistory.Add(CurrentTime);
        
    // 保持历史记录在指定长度
    if (PositionHistory.Num() > POSITION_HISTORY_LENGTH)
    {
        PositionHistory.RemoveAt(0);
        TimeHistory.RemoveAt(0);
    }
        
    // 检测抛投动作并获取速度和方向
    FVector ThrowVelocity, ThrowDirection;
    // 检测抛投动作
    if (DetectThrowGesture(ThrowVelocity, ThrowDirection))
    {
        // 检测到抛投动作，可以在这里触发事件或者调用函数
        UE_LOG(LogTemp, Log, TEXT("Throw Detected!"));
        // 广播抛投事件
        OnThrowDetected.Broadcast(ThrowVelocity, ThrowDirection);
            
        // 清空历史数据，准备下一次检测
        PositionHistory.Empty();
        TimeHistory.Empty();
    }
   
}

bool UThrowDetectorComponent::DetectThrowGesture(FVector& OutVelocity, FVector& OutDirection)
{
    if (PositionHistory.Num() < POSITION_HISTORY_LENGTH)
    {
        return false;
    }

    OutVelocity = CalculateVelocity();
    float Speed = OutVelocity.Size();

    if (Speed < MinThrowVelocity)
    {
        return false;
    }

    OutDirection = OutVelocity.GetSafeNormal();

    float VerticalAngle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(OutDirection, FVector::UpVector))
    );

    bool bValidDirection = VerticalAngle >= MinVerticalAngle && VerticalAngle <= MaxVerticalAngle;
    bool bHasAcceleration = CheckAcceleration();

    return bValidDirection && bHasAcceleration;
}

FVector UThrowDetectorComponent::CalculateVelocity()
{
	// 使用最近两个点计算速度
	int32 LastIndex = PositionHistory.Num() - 1;
	FVector DeltaPosition = PositionHistory[LastIndex] - PositionHistory[LastIndex - 1];
	float DeltaTime = TimeHistory[LastIndex] - TimeHistory[LastIndex - 1];
	
	return DeltaPosition / DeltaTime;
}

bool UThrowDetectorComponent::CheckAcceleration()
{
	// 检查是否存在明显的加速过程
	float PrevSpeed = 0.0f;
	bool bFoundAcceleration = false;
	
	for (int32 i = 1; i < PositionHistory.Num(); i++)
	{
		FVector DeltaPosition = PositionHistory[i] - PositionHistory[i-1];
		float DeltaTime = TimeHistory[i] - TimeHistory[i-1];
		float CurrentSpeed = DeltaPosition.Size() / DeltaTime;
		
		if (CurrentSpeed > PrevSpeed * 1.5f) // 速度增加超过50%
		{
			bFoundAcceleration = true;
			break;
		}
		
		PrevSpeed = CurrentSpeed;
	}
	
	return bFoundAcceleration;
}