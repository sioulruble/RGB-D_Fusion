#include "TrackerUpdaterComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include <openvr.h>

#include "IntVectorTypes.h"


UTrackerUpdaterComponent::UTrackerUpdaterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTrackerUpdaterComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize OpenVR!"));
	}
}

void UTrackerUpdaterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TrackerMesh) return;

	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, poses, vr::k_unMaxTrackedDeviceCount);
	countTracker = 0;
	TrackerPositions.Empty();
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
	{
		if (vr::VRSystem()->IsTrackedDeviceConnected(i))
		{
			vr::ETrackedDeviceClass DeviceClass = vr::VRSystem()->GetTrackedDeviceClass(i);
			
			if (DeviceClass == vr::TrackedDeviceClass_HMD && poses[i].bPoseIsValid)
            {
                const vr::HmdMatrix34_t& hmdMat = poses[i].mDeviceToAbsoluteTracking;
                SteamVRHDMPos = FVector(-hmdMat.m[2][3] * 100.f,hmdMat.m[0][3] * 100.f,hmdMat.m[1][3] * 100.f);
				if (PlayerPawn)
				{
					// Get the position of the VR Headset in the Scene
					TArray<UCameraComponent*> CameraComponents;
					PlayerPawn->GetComponents<UCameraComponent>(CameraComponents);
             
					// Compute the Mesh Position Minus the Camera Position to align the two elements
					for (UCameraComponent* CameraComp : CameraComponents)
					{
						HMDOffset = CameraComp->GetComponentLocation() - SteamVRHDMPos;
						break;
					}
				}
				TrackerMeshHMD->SetWorldLocation(SteamVRHDMPos + HMDOffset);
            } else if (poses[i].bPoseIsValid && DeviceClass == vr::TrackedDeviceClass_GenericTracker)
			{
				const vr::HmdMatrix34_t& mat = poses[i].mDeviceToAbsoluteTracking;

				FVector Pos(-mat.m[2][3] * 100.f,mat.m[0][3] * 100.f,mat.m[1][3] * 100.f);
            	FVector FinalPos = Pos + HMDOffset;
            	TrackerPositions.Add(FinalPos);
            	
            	FMatrix M(
            		FPlane( mat.m[0][0], -mat.m[2][0], mat.m[1][0], 0),
            		FPlane( mat.m[0][2], -mat.m[2][2], mat.m[1][2], 0),
            		FPlane( mat.m[0][1], -mat.m[2][1], mat.m[1][1], 0),
            		FPlane(0, 0, 0, 1));

            	FRotator Rot = M.Rotator();
			}
		}
	}
	if (TrackerPositions.Num() == 3)
	{
		TrackerPositions.Sort([](const FVector& A, const FVector& B)
		{
			return A.Y < B.Y;
		});
		TrackerMesh->SetWorldLocation(TrackerPositions[0]); // Left Elbow
		TrackerMesh2->SetWorldLocation(TrackerPositions[1]); // Waist
		TrackerMesh3->SetWorldLocation(TrackerPositions[2]); // Right Elbow
		
		TrackerMesh->Rename(TEXT("Left_Elbow"));
		TrackerMesh2->Rename(TEXT("Waist"));
		TrackerMesh3->Rename(TEXT("Right_Elbow"));
	}
}

