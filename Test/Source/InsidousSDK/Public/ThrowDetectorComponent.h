#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThrowDetectorComponent.generated.h"

// 声明抛投事件的代理
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThrowDetected, FVector, ThrowVelocity, FVector, ThrowDirection);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INSIDOUSSDK_API UThrowDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UThrowDetectorComponent();

    UPROPERTY(BlueprintAssignable, Category = "VR|Throw")
    FOnThrowDetected OnThrowDetected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Detection")
    float MinThrowVelocity = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Detection")
    float MinVerticalAngle = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Detection")
    float MaxVerticalAngle = 150.0f;

    UFUNCTION(BlueprintCallable, Category = "Throw Detection")
	void SetCheckComponent(USceneComponent* component);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    static const int32 POSITION_HISTORY_LENGTH = 10;
    
    UPROPERTY()
    class USceneComponent* comp;
    
    TArray<FVector> PositionHistory;
    TArray<float> TimeHistory;
    
    float TimeSinceLastSample = 0.0f;
    
    bool DetectThrowGesture(FVector& OutVelocity, FVector& OutDirection);
    FVector CalculateVelocity();
    bool CheckAcceleration();
};