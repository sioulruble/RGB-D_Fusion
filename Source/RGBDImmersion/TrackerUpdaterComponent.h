// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrackerUpdaterComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RGBDIMMERSION_API UTrackerUpdaterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTrackerUpdaterComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracker")
	UStaticMeshComponent* TrackerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracker")
	UStaticMeshComponent* TrackerMesh2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracker")
	UStaticMeshComponent* TrackerMesh3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracker")
	UStaticMeshComponent* TrackerMeshHMD;
	
	UPROPERTY()
	TArray<FVector> TrackerPositions;
	
	int countTracker = 0;
	UPROPERTY()
	FVector SteamVRHDMPos;
	UPROPERTY()
	FVector HMDOffset;
	UPROPERTY()
	APawn* PlayerPawn;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
