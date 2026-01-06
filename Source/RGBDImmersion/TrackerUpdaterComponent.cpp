#include "TrackerUpdaterComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <openvr.h>

UTrackerUpdaterComponent::UTrackerUpdaterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTrackerUpdaterComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	vr::EVRInitError Error = vr::VRInitError_None;
	vr::VR_Init(&Error, vr::VRApplication_Scene);

	if (Error != vr::VRInitError_None)
	{
		UE_LOG(LogTemp, Error, TEXT("OpenVR init failed"));
		return;
	}

	Serial_LeftElbow  = TEXT("LHR-2DF36558");
	Serial_Waist      = TEXT("LHR-4D18CA04");
	Serial_RightElbow = TEXT("LHR-00BDC466");

	UE_LOG(LogTemp, Warning, TEXT("[YAML FINAL] LeftElbow  = '%s'"), *Serial_LeftElbow);
	UE_LOG(LogTemp, Warning, TEXT("[YAML FINAL] RightElbow = '%s'"), *Serial_RightElbow);
	UE_LOG(LogTemp, Warning, TEXT("[YAML FINAL] Waist      = '%s'"), *Serial_Waist);
}

void UTrackerUpdaterComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	vr::TrackedDevicePose_t Poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(
		vr::TrackingUniverseStanding,
		0,
		Poses,
		vr::k_unMaxTrackedDeviceCount
	);

	for (int32 i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
	{
		if (!vr::VRSystem()->IsTrackedDeviceConnected(i))
			continue;

		if (!Poses[i].bPoseIsValid)
			continue;

		vr::ETrackedDeviceClass DeviceClass =
			vr::VRSystem()->GetTrackedDeviceClass(i);

		const vr::HmdMatrix34_t& Mat = Poses[i].mDeviceToAbsoluteTracking;

		FVector Pos(
			-Mat.m[2][3] * 100.f,
			 Mat.m[0][3] * 100.f,
			 Mat.m[1][3] * 100.f
		);

		FVector X(Mat.m[0][0], Mat.m[1][0], Mat.m[2][0]);
		FVector Y(Mat.m[0][1], Mat.m[1][1], Mat.m[2][1]);
		FVector Z(Mat.m[0][2], Mat.m[1][2], Mat.m[2][2]);

		FMatrix M;
		M.SetAxes(&X, &Y, &Z);

		FRotator Rot = FQuat(M).Rotator();

		if (DeviceClass == vr::TrackedDeviceClass_HMD)
		{
			if (PlayerPawn)
			{
				TArray<UCameraComponent*> Cameras;
				PlayerPawn->GetComponents(Cameras);
				if (Cameras.Num() > 0)
					HMDOffset = Cameras[0]->GetComponentLocation() - Pos;
			}

			if (TrackerMeshHMD)
				TrackerMeshHMD->SetWorldLocation(Pos + HMDOffset);

			continue;
		}

		if (DeviceClass != vr::TrackedDeviceClass_GenericTracker)
			continue;

		char SerialBuffer[vr::k_unMaxPropertyStringSize];
		vr::VRSystem()->GetStringTrackedDeviceProperty(
			i,
			vr::Prop_SerialNumber_String,
			SerialBuffer,
			vr::k_unMaxPropertyStringSize
		);

		FString DeviceSerial = UTF8_TO_TCHAR(SerialBuffer);
		DeviceSerial = DeviceSerial.TrimStartAndEnd();

		FVector FinalPos = Pos + HMDOffset;

		if (DeviceSerial == Serial_LeftElbow )
		{
			TrackerMesh->SetWorldLocation(FinalPos);
			TrackerMesh->SetWorldRotation(Rot);
		}
		else if (DeviceSerial == Serial_Waist )
		{
			TrackerMesh2->SetWorldLocation(FinalPos);
			TrackerMesh2->SetWorldRotation(Rot);
		}
		else if (DeviceSerial == Serial_RightElbow )
		{
			TrackerMesh3->SetWorldLocation(FinalPos);
			TrackerMesh3->SetWorldRotation(Rot);
		}
	}
}
