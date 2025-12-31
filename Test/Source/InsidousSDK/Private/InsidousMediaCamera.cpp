// Fill out your copyright notice in the Description page of Project Settings.


#include "InsidousMediaCamera.h"

// Sets default values
AInsidousMediaCamera::AInsidousMediaCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
	CameraRoot->SetupAttachment(RootComponent);

	Camera2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Camera2D"));
	Camera2D->SetupAttachment(CameraRoot);

}

// Called when the game starts or when spawned
void AInsidousMediaCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInsidousMediaCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

