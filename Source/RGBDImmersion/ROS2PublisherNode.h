// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CheckBox.h"

#include "ROS2Publisher.h"
#include <Msgs/ROS2String.h>

#include "ROS2PublisherNode.generated.h"


UCLASS()
class RGBDIMMERSION_API AROS2PublisherNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AROS2PublisherNode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UROS2NodeComponent* Node = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UROS2Publisher* LoopPublisher = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString McRtcTopicName = TEXT("unreal/data");

	UFUNCTION()
	void PublishJointState(UROS2GenericMsg* InMessage);

	UPROPERTY()
	APawn* PlayerPawn;

	UPROPERTY()
	FVector LeftElbowPos;
	UPROPERTY()
	FVector RightElbowPos;
	UPROPERTY()
	FVector WaistPos;
	UPROPERTY()
	FVector RightHandPos;
	UPROPERTY()
	FRotator LeftElbowRot;
	UPROPERTY()
	FRotator RightElbowRot;
	UPROPERTY()
	FRotator WaistRot;
	UPROPERTY()
	FRotator RightHandRot;

	UPROPERTY()
	bool IsRobotMoving = false;
	UPROPERTY()
	UCheckBox* MoveRobot = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
