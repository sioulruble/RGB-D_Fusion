// Fill out your copyright notice in the Description page of Project Settings.

#include "Math/UnrealMathUtility.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "ROS2PublisherNode.h"

#include "MotionControllerComponent.h"

// Sets default values
AROS2PublisherNode::AROS2PublisherNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Node = CreateDefaultSubobject<UROS2NodeComponent>(TEXT("ROS2NodeComponent"));
	Node->Name = TEXT("unreal_publisher_node");
}

// Called when the game starts or when spawned
void AROS2PublisherNode::BeginPlay()
{
	Super::BeginPlay();
	Node->Init();

	ROS2_CREATE_LOOP_PUBLISHER_WITH_QOS(Node, this, McRtcTopicName, UROS2Publisher::StaticClass(), UROS2StringMsg::StaticClass(), 1000, &AROS2PublisherNode::PublishJointState, UROS2QoS::Default, LoopPublisher);

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

// Called every frame
void AROS2PublisherNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MoveRobot)
    {
       if (PlayerPawn)
       {
          // Navigate through the Widget of the Pawn to find the UI
          TArray<UWidgetComponent*> WidgetComponents;
          PlayerPawn->GetComponents<UWidgetComponent>(WidgetComponents);
          for (UWidgetComponent* WidgetComp : WidgetComponents)
          {
             FString name = WidgetComp->GetName();
             if (WidgetComp->GetName().Contains(TEXT("Widget")))
             {
                UUserWidget* Widget = WidgetComp->GetUserWidgetObject();
                if (!Widget) continue;
    
                // Save the useful Widget
                MoveRobot = Cast<UCheckBox>(Widget->GetWidgetFromName(TEXT("MoveRobot")));
                if (MoveRobot) {
                	UE_LOG(LogTemp, Warning, TEXT("MoveRobot Checkbox found"));
					break;
                }
             }
          }
       }
    }
    else
    {
	    if (MoveRobot) {
	    	if (MoveRobot->IsChecked() != IsRobotMoving) {
	    		UE_LOG(LogTemp, Warning, TEXT("Robot moving: %s"), IsRobotMoving ? TEXT("true") : TEXT("false"));
	    		IsRobotMoving = MoveRobot->IsChecked();
	    	}
	    }
    }
}


void AROS2PublisherNode::PublishJointState(UROS2GenericMsg* InMessage) {
	FROSString msg;
	msg.Data = FString::Printf(TEXT("{"));

	if (IsRobotMoving) {
		msg.Data = "\"Running\": 1,";
	} else {
		msg.Data = "\"Running\": 0,";
	}

	float RightHandRoll = 0, RightHandPitch = 0, RightHandYaw = 0;
	float RightHandX = 0, RightHandY = 0, RightHandZ = 0;

	if (PlayerPawn) {
		// Get the position of the VR Headset in the Scene
		TArray<UCameraComponent*> CameraComponents;
		TArray<UMotionControllerComponent*> MotionComponents;
		PlayerPawn->GetComponents<UCameraComponent>(CameraComponents);
		PlayerPawn->GetComponents<UMotionControllerComponent>(MotionComponents);
		
		// Compute the Mesh Position Minus the Camera Position to align the two elements
		for (UCameraComponent* CameraComp : CameraComponents) {
			for (UMotionControllerComponent* MotionComp : MotionComponents)
			{
				if (MotionComp->GetName() == "MotionControllerRight")
				{
					FRotator rot = CameraComp->GetRelativeRotation();
					FVector HandPos = MotionComp->GetComponentLocation();
					FRotator HandRot = MotionComp->GetComponentRotation();

					float CameraRoll  =  rot.Roll  * PI / 180.0f, CameraPitch = -rot.Pitch * PI / 180.0f, CameraYaw   = -rot.Yaw   * PI / 180.0f;
					RightHandRoll  =  HandRot.Roll  * PI / 180.0f, RightHandPitch = -HandRot.Pitch * PI / 180.0f, RightHandYaw   = -HandRot.Yaw   * PI / 180.0f;
					RightHandX  =  HandPos.X, RightHandY  =  HandPos.Y, RightHandZ  =  HandPos.Z;

					msg.Data += FString::Printf(
						TEXT("\"NECK_Y\": %f, \"NECK_P\": %f, \"NECK_R\": %f, "),
						CameraYaw, CameraPitch, CameraRoll
					);
					break;
				}
			}
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		PlayerPawn->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

		for (UStaticMeshComponent* StaticComp : StaticMeshComponents)
		{
			if (StaticComp->GetName() == "Left_Elbow") {
				LeftElbowPos = StaticComp->GetComponentLocation();
				LeftElbowRot = StaticComp->GetComponentRotation();
			} else if (StaticComp->GetName() == "Right_Elbow") {
				RightElbowPos = StaticComp->GetComponentLocation();
				RightElbowRot = StaticComp->GetComponentRotation();
			} else if (StaticComp->GetName() == "Waist") {
				WaistPos = StaticComp->GetComponentLocation();
				WaistRot = StaticComp->GetComponentRotation();
			}
		}
	}
	WaistPos.Z = 0;
	FRotator RightHandRotation(RightHandPitch,RightHandYaw, RightHandRoll);
	FVector RightHandTrans(RightHandX, RightHandY, RightHandZ);

	FTransform RightHandTransform(RightHandRotation, RightHandTrans);
	FTransform WaistTransform(WaistRot, WaistPos);
	FTransform LeftElbowTransform(LeftElbowRot, LeftElbowPos);
	FTransform RightElbowTransform(RightElbowRot, RightElbowPos);
	

	FTransform WaistInv = WaistTransform.Inverse();

	FTransform LeftElbowLocal = LeftElbowTransform * WaistInv;
	FTransform RightElbowLocal = RightElbowTransform * WaistInv;
	FTransform RightHandLocal = RightHandTransform.GetRelativeTransform(WaistTransform);

	FVector LeftElbowLocalPos = LeftElbowLocal.GetLocation();
	FRotator LeftElbowLocalRot = LeftElbowLocal.Rotator();

	FVector RightElbowLocalPos = RightElbowLocal.GetLocation();
	FRotator RightElbowLocalRot = RightElbowLocal.Rotator();

	FVector RightHandLocalPos = RightHandLocal.GetLocation();
	FRotator RightHandLocalRot = RightHandLocal.Rotator();

	UE_LOG(LogTemp, Warning, TEXT("Hand World: (%f, %f, %f) | Waist World: (%f, %f, %f)"),
		RightHandX, RightHandY, RightHandZ,
		WaistPos.X, WaistPos.Y, WaistPos.Z);

	UE_LOG(LogTemp, Warning, TEXT("Hand Local: (%f, %f, %f)"),
		RightHandLocal.GetLocation().X,
		RightHandLocal.GetLocation().Y,
		RightHandLocal.GetLocation().Z);

	
	msg.Data += FString::Printf(
	TEXT("\"RHAND_X\": %f, \"RHAND_Y\": %f, \"RHAND_Z\": %f,")
		TEXT("\"RHAND_YAW\": %f, \"RHAND_PITCH\": %f, \"RHAND_ROLL\": %f,"),
		-RightHandTrans.Y / 100, -RightHandTrans.X / 100, RightHandTrans.Z / 100,
		RightHandLocalRot.Yaw, RightHandLocalRot.Pitch, RightHandLocalRot.Roll);
	
	msg.Data += FString::Printf(TEXT("}"));
	CastChecked<UROS2StringMsg>(InMessage)->SetMsg(msg);
}
