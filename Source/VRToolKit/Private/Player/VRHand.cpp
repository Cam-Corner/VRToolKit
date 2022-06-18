// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRHand.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MotionControllerComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Online/NetworkHelpers.h"
#include "Items/VRGrabComponent.h"
#include "DrawDebugHelpers.h"
#include "Items/VRItem.h"
#include "Online/PhysicsReplicationComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Components/SphereComponent.h"
#include "ClimbingSystem/EnvironmentGrabComponent.h"
#include "Player/VRCharacter.h"

// Sets default values
AVRHand::AVRHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent
	_RootComp = CreateDefaultSubobject<USceneComponent>("RootComp");
	SetRootComponent(_RootComp);

	_OnCalculateCustomPhysics.BindUObject(this, &AVRHand::CustomPhysics);

	//Motion Controller Component
	_MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	_MotionControllerComp->SetupAttachment(GetRootComponent());

	//Physics Hand Collision
	{
		_PhysicsHandCollision = CreateDefaultSubobject<UBoxComponent>("PhysicsHandCollision");
		_PhysicsHandCollision->SetupAttachment(GetRootComponent());
		_PhysicsHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		_PhysicsHandCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		_PhysicsHandCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		_PhysicsHandCollision->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
		_PhysicsHandCollision->SetSimulatePhysics(true);
	}

	//VR Controller Mesh
	{
		_ControllerSM = CreateDefaultSubobject<UStaticMeshComponent>("ControllerMesh");
		_ControllerSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_ControllerSM->SetupAttachment(_MotionControllerComp);

		_LeftPhysicsHandSM = CreateDefaultSubobject<UStaticMeshComponent>("LeftPhysicsHandSM");
		_LeftPhysicsHandSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_LeftPhysicsHandSM->SetupAttachment(_PhysicsHandCollision);

		_RightPhysicsHandSM = CreateDefaultSubobject<UStaticMeshComponent>("RightPhysicsHandSM");
		_RightPhysicsHandSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_RightPhysicsHandSM->SetupAttachment(_PhysicsHandCollision);

		static ConstructorHelpers::FObjectFinder<UStaticMesh> ControllerStaticMesh(
			TEXT("/Engine/VREditor/Devices/Oculus/OculusControllerMesh"));
		if (ControllerStaticMesh.Succeeded())
		{
			_ControllerSM->SetStaticMesh(ControllerStaticMesh.Object);
			_LeftPhysicsHandSM->SetStaticMesh(ControllerStaticMesh.Object);
			_RightPhysicsHandSM->SetStaticMesh(ControllerStaticMesh.Object);
			
		}

		/*static ConstructorHelpers::FObjectFinder<UMaterialInstance> ControllerMaterial(
			TEXT("/Game/Content/Materials/Colours/M_Translucent_Blue"));
		if (ControllerMaterial.Succeeded())
		{
			_ControllerSM->SetMaterial(0, ControllerMaterial.Object);
		}*/
	}

	_PRC = CreateDefaultSubobject<UPhysicsReplicationComponent>("PRC");

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");

	//Grab Sphere
	{
		_GrabSphere = CreateDefaultSubobject<USphereComponent>("GrabSphere");
		_GrabSphere->SetupAttachment(_PhysicsHandCollision);
		_GrabSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		_GrabSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		_GrabSphere->SetSphereRadius(5.f);
		_GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRHand::GrabSphereOverlapBegin);
		_GrabSphere->OnComponentEndOverlap.AddDynamic(this, &AVRHand::GrabSphereOverlapEnd);
	}
}

// Called when the game starts or when spawned
void AVRHand::BeginPlay()
{
	Super::BeginPlay();

	/*if (GetOwner() != UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		//_ControllerSM->SetVisibility(false);
		_PhysicsHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		_PhysicsHandCollision->SetSimulatePhysics(false);
		_CorrectResponseContainer = _PhysicsHandCollision->GetCollisionResponseToChannels();
	}*/

	//_PRC->SetReplicatedPhysicsObject(_PhysicsHandCollision);

	_PHC->SetReplicatedPhysicsObject(_PhysicsHandCollision);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	_PHC->SetTargetObject(_MotionControllerComp);
	_PHC->SetMatchTarget(true);
	_PHC->SetPlayerControllerOwner(Cast<APlayerController>(GetOwner()));
	_LastHandLocation = _MotionControllerComp->GetComponentLocation();
}

// Called every frame
void AVRHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (!_ComponentHeld)
		//MovePhysicsHand(DeltaTime);

	/*if (GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		

		_ReplicatedHandTransformTimer -= DeltaTime;

		if (_ReplicatedHandTransformTimer <= 0)
		{
			if (_PhysicsHandCollision && _ControllerSM)
			{
				float Dist = FVector::Dist(_PhysicsHandCollision->GetComponentLocation(),
					_ControllerSM->GetComponentLocation());
				if (Dist > 50)
				{
					_PhysicsHandCollision->SetWorldLocation(_ControllerSM->GetComponentLocation());
				}

				_ReplicatedHandTransformTimer = 0.1f;
				Server_NewHandTransform(_PhysicsHandCollision->GetComponentLocation(),
					_PhysicsHandCollision->GetComponentRotation(), _ControllerSM->GetComponentLocation(),
					_ControllerSM->GetComponentRotation());
			}
		}
	}
	else if (GetLocalRole() >= ENetRole::ROLE_SimulatedProxy && _PhysicsHandCollision)
	{
		{
			FVector NewLocation;
			FVector CurrentLoc = _PhysicsHandCollision->GetComponentLocation();
			NewLocation.X = FMath::FInterpTo(CurrentLoc.X, _LastKNownHandLocation.X, DeltaTime, 15.f);
			NewLocation.Y = FMath::FInterpTo(CurrentLoc.Y, _LastKNownHandLocation.Y, DeltaTime, 15.f);
			NewLocation.Z = FMath::FInterpTo(CurrentLoc.Z, _LastKNownHandLocation.Z, DeltaTime, 15.f);
			_PhysicsHandCollision->SetWorldLocation(NewLocation);

			FRotator NewRotation;
			FRotator CurrentRot = _PhysicsHandCollision->GetComponentRotation();
			NewRotation = FMath::RInterpTo(CurrentRot, _LastKNownHandRotation, DeltaTime, 10.f);
			_PhysicsHandCollision->SetWorldRotation(NewRotation);
		}

		{
			//Hard Set the tracking hands because they are invisible so not important about it looking unsmooth
			/*FVector NewLocation;
			FVector CurrentLoc = _ControllerSM->GetComponentLocation();
			NewLocation.X = FMath::FInterpTo(CurrentLoc.X, _LastKNownTrackingHandLocation.X, DeltaTime, 15.f);
			NewLocation.Y = FMath::FInterpTo(CurrentLoc.Y, _LastKNownTrackingHandLocation.Y, DeltaTime, 15.f);
			NewLocation.Z = FMath::FInterpTo(CurrentLoc.Z, _LastKNownTrackingHandLocation.Z, DeltaTime, 15.f);
			_MotionControllerComp->SetWorldLocation(_LastKNownTrackingHandLocation);

			/*FRotator NewRotation;
			FRotator CurrentRot = _ControllerSM->GetComponentRotation();
			NewRotation = FMath::RInterpTo(CurrentRot, _LastKNownTrackingHandRotation, DeltaTime, 10.f);
			_MotionControllerComp->SetWorldRotation(_LastKNownTrackingHandRotation);
		}
	}*/

	if (_CharacterAttachedTo && _GrabbedEGC)
	{
		UpdateHandClimbInfo();
	}
}

void AVRHand::PhysicsTick_Implementation(float SubsetDeltaTime)
{
	if (!_PRC)
		return;

	FBodyInstance* PH_BI = _PhysicsHandCollision->GetBodyInstance();
	if (GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{	
		FTransform WorldT = PH_BI->GetUnrealWorldTransform_AssumesLocked();

		FTransform HandTransform = _MotionControllerComp->GetComponentTransform();

		{
			//Linear Force
			FVector From = WorldT.GetLocation();
			FVector To = HandTransform.GetLocation();

			FVector Force = _LocPD.GetForce(SubsetDeltaTime, From, To);
			PH_BI->AddForce(Force * PH_BI->GetBodyMass(), false);
		}

		{
			FVector Torque = _RotPD.GetTorque(SubsetDeltaTime, WorldT.GetRotation(),
				HandTransform.GetRotation(), FMath::RadiansToDegrees(PH_BI->GetUnrealWorldAngularVelocityInRadians()),
				FVector(0, 0, 0), WorldT);

			PH_BI->AddTorqueInRadians(FMath::DegreesToRadians(Torque), false, true);
		}

		FPhysicsFrame PF;
		PF._Location = WorldT.GetLocation();
		PF._Rotation = WorldT.GetRotation();
		PF._Linear_Velocity = PH_BI->GetUnrealWorldVelocity();
		PF._Angular_Velocity = PH_BI->GetUnrealWorldAngularVelocityInRadians();
		_PRC->SendPhysicsTick(PF);
	}
	else
	{
		FPhysicsFrame PF = _PRC->GetNewPhysicsFrame();

		FTransform T;
		T.SetLocation(PF._Location);
		T.SetRotation(PF._Rotation);
		PH_BI->SetBodyTransform(T, ETeleportType::ResetPhysics);

		PH_BI->SetLinearVelocity(PF._Linear_Velocity, false);
		PH_BI->SetAngularVelocityInRadians(PF._Linear_Velocity, false);
	}
}

void AVRHand::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	PhysicsTick_Implementation(DeltaTime);
}

void AVRHand::MoveHandOutsideOfHeldObject()
{
	if (!_RootComp || !_MotionControllerComp || !_PhysicsHandCollision)
		return;

	FVector EndLoc = _MotionControllerComp->GetComponentLocation();
	FVector StartLoc = _RootComp->GetComponentLocation() + (GetMotionControllerMoveForwardDir() * 10);
	StartLoc.Z = EndLoc.Z;
	FVector Dir = EndLoc - StartLoc;
	ExtraMaths::CorrectNormalizedVector(Dir);
	EndLoc -= Dir * 10;

	_PhysicsHandCollision->SetWorldLocation(StartLoc);
	_PhysicsHandCollision->SetWorldLocation(EndLoc, true);
	_PhysicsHandCollision->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	_PhysicsHandCollision->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void AVRHand::UpdateHandClimbInfo()
{
	if (_GrabbedEGC)
	{
		_ClimbingInfo._EGC_CurrentLoc = _GrabbedEGC->GetComponentLocation();
		_ClimbingInfo._EGC_MoveDiff = _ClimbingInfo._EGC_CurrentLoc - _ClimbingInfo._EGC_StartLocation;
	}

	_ClimbingInfo._MC_CurrentLoc = _MotionControllerComp->GetComponentLocation();
	_ClimbingInfo._MC_MoveDiff = _MotionControllerComp->GetComponentLocation() - (_ClimbingInfo._GrabbedLocation + _ClimbingInfo._EGC_MoveDiff);

	_LastHandLocation = _MotionControllerComp->GetComponentLocation();
}

void AVRHand::GrabSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(3, 1.0, FColor::Red, "DSASDF ");
	if (UVRGrabComponent* GC = Cast<UVRGrabComponent>(OtherComp))
	{
		_GrabCompArray.Add(GC);
	}
	else if (UEnvironmentGrabComponent* EGC = Cast<UEnvironmentGrabComponent>(OtherComp))
	{
		_EnvironmentGrabCompArray.Add(EGC);
	}
}

void AVRHand::GrabSphereOverlapEnd(	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UVRGrabComponent* GC = Cast<UVRGrabComponent>(OtherComp))
	{
		_GrabCompArray.Remove(GC);
	}
	else if (UEnvironmentGrabComponent* EGC = Cast<UEnvironmentGrabComponent>(OtherComp))
	{
		_EnvironmentGrabCompArray.Remove(EGC);
	}
}

void AVRHand::IsRightHand(bool bRightHand)
{
	_bRightHand = bRightHand;

	if (bRightHand)
	{
		if (_RightPhysicsHandSM)
		{
			_PhysicsHandSM = _RightPhysicsHandSM;
			_RightPhysicsHandSM->SetVisibility(true);
		}
		
		if (_LeftPhysicsHandSM)
			_LeftPhysicsHandSM->SetVisibility(false);

	}
	else
	{
		if (_LeftPhysicsHandSM)
		{
			_PhysicsHandSM = _LeftPhysicsHandSM;
			_LeftPhysicsHandSM->SetVisibility(true);
		}

		if (_RightPhysicsHandSM)
			_RightPhysicsHandSM->SetVisibility(false);
	}

	if (!_PhysicsHandSM || !_ControllerSM || !_MotionControllerComp)
		return;

	if (bRightHand)
	{
		_ControllerSM->SetRelativeScale3D(FVector(1.f, -1.f, 1.f));
		_MotionControllerComp->MotionSource = "Right";
	}
	else
	{		
		_ControllerSM->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		_MotionControllerComp->MotionSource = "Left";
	}
}

FVector AVRHand::GetMotionControllerMoveForwardDir()
{
	if (!_RootComp || !_MotionControllerComp || !_PhysicsHandCollision)
		return FVector::ZeroVector;

	FVector StartLoc = _RootComp->GetComponentLocation();
	FVector EndLoc = _MotionControllerComp->GetComponentLocation();
	FVector Dir = EndLoc - StartLoc;
	Dir.Z = 0;
	ExtraMaths::CorrectNormalizedVector(Dir);

	return Dir;
}

void AVRHand::GripPressed()
{
	if (_bDoneGrab)
		return;

	_bDoneGrab = true;


	if (_EnvironmentGrabCompArray.Num() > 0)
	{
		if (!_EnvironmentGrabCompArray[0])
			return;

		_GrabbedEGC = _EnvironmentGrabCompArray[0];
		_GrabbedEGC->HandGrabbed(this);
		_ClimbingInfo._GrabbedLocation = _MotionControllerComp->GetComponentLocation();
		_ClimbingInfo._EGC_StartLocation = _GrabbedEGC->GetComponentLocation();

		if (_CharacterAttachedTo)
			_CharacterAttachedTo->HandGrabbedClimbingPoint(!GetIsRightHand(), true);

	}
	else
	{
		AttemptItemGrab();
	}

	/*if (_LeftPhysicsHandSM)
	{
		_PhysicsHandSM = _LeftPhysicsHandSM;
		_LeftPhysicsHandSM->SetVisibility(false);
	}

	if (_RightPhysicsHandSM)
	{
		_RightPhysicsHandSM->SetVisibility(false);
	}

	if (_ControllerSM)
		_ControllerSM->SetVisibility(true);*/
}

void AVRHand::GripReleased()
{
	if (!_bDoneGrab)
		return;

	_bDoneGrab = false;

	if (_ComponentHeld)
	{
		_ComponentHeld->ReleaseComponent();
		_ComponentHeld = NULL;

		_PhysicsHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MoveHandOutsideOfHeldObject();
		_PhysicsHandCollision->SetSimulatePhysics(true);
	
		Server_DroppedGrabbedComponent();		
	}

	if (_GrabbedEGC)
	{
		_GrabbedEGC->HandReleased(this);
		_GrabbedEGC = NULL;
		if (_CharacterAttachedTo)
			_CharacterAttachedTo->HandGrabbedClimbingPoint(!GetIsRightHand(), false);
	}

	/*if (_bRightHand)
	{
		if (_RightPhysicsHandSM)
		{
			_RightPhysicsHandSM->SetVisibility(true);
		}
	}
	else
	{
		if (_LeftPhysicsHandSM)
		{
			_PhysicsHandSM = _LeftPhysicsHandSM;
			_LeftPhysicsHandSM->SetVisibility(true);
		}
	}

	if (_ControllerSM)
		_ControllerSM->SetVisibility(false);*/

}

void AVRHand::TopButtonPressed(bool bPressed)
{
	if (_ComponentHeld)
	{
		_ComponentHeld->TopButtonPressed(bPressed);
	}
}

void AVRHand::BottomButtonPressed(bool bPressed)
{
	if (_ComponentHeld)
	{
		_ComponentHeld->BottomButtonPressed(bPressed);
	}
}

void AVRHand::TriggerPressed(float Value)
{
	if (_ComponentHeld)
	{
		_ComponentHeld->TriggerPressed(Value);
	}
}

const FTransform AVRHand::GetTrackingHandTransform()
{
	if (!_MotionControllerComp)
		return FTransform();

	return _MotionControllerComp->GetComponentTransform();
}

void AVRHand::UpdateOwner(AController* PC)
{
	SetOwner(PC);

	if (!_PHC)
		return;

	_PHC->SetReplicatedPhysicsObject(_PhysicsHandCollision);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	_PHC->SetTargetObject(_MotionControllerComp);
	_PHC->SetMatchTarget(true);
	_PHC->SetPlayerControllerOwner(PC);
}

FTransform AVRHand::GetPhysicsObjectTransform()
{
	if(!_PhysicsHandCollision)
		return FTransform();

	return _PhysicsHandCollision->GetComponentTransform();
}

void AVRHand::MovePhysicsHand(float DeltaTime)
{
	if (!_PhysicsHandSM || !_ControllerSM || !_PhysicsHandCollision)
		return;

	if (_PhysicsHandCollision->GetBodyInstance())
	{
		_PhysicsHandCollision->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}
	{
		//Linear Force
		/*FVector From = _PhysicsHandSM->GetComponentLocation();
		FVector To = _ControllerSM->GetComponentLocation();

		FVector Force = LocPD.GetForce(DeltaTime, From, To);
		_PhysicsHandCollision->AddForce(Force * _PhysicsHandCollision->GetMass());

		//Angular Force
		FQuat FQ = _PhysicsHandSM->GetComponentQuat();
		FQuat TQ = _ControllerSM->GetComponentQuat();
		FVector Vel = _PhysicsHandCollision->GetPhysicsAngularVelocityInRadians();
		FVector IT = _PhysicsHandCollision->GetInertiaTensor();

		FVector Torque = RotPD.GetTorque(DeltaTime, FQ, TQ, Vel, IT, _PhysicsHandCollision->GetComponentTransform());
		_PhysicsHandCollision->AddTorqueInRadians(Torque * IT /** _PhysicsHandCollision->GetMass());*/

		/*float DesiredVel = 500;
		float Q = 500;
		float P = 100;

		FRotator R = _MotionControllerComp->GetComponentRotation() - _PhysicsHandCollision->GetComponentRotation();
		FVector W = FVector(DesiredVel, DesiredVel, DesiredVel) - _PhysicsHandCollision->GetPhysicsAngularVelocityInDegrees();
		FVector Torque;
		Torque = Q * R.Roll * FVector(1, 0, 0) + P * W * FVector(1, 0, 0);
		_PhysicsHandCollision->AddTorqueInDegrees(Torque);

		Torque = Q * R.Pitch * FVector(0, 1, 0) + P * W * FVector(0, 1, 0);
		_PhysicsHandCollision->AddTorqueInDegrees(Torque);

		Torque = Q * R.Yaw * FVector(0, 0, 1) + P * W * FVector(0, 0, 1);
		_PhysicsHandCollision->AddTorqueInDegrees(Torque);*/
	}
}

void AVRHand::Client_ItemPickupInvalid_Implementation()
{
	GripReleased();
}

void AVRHand::Server_GrabbedComponent_Implementation(UVRGrabComponent* GrabComp)
{	
	if (!GrabComp)
		return;

	if (!GrabComp->CanGrabComponent())
	{
		Client_ItemPickupInvalid();
	}
	else
	{
		if (GrabComp->GrabComponent(this))
		{
			_ComponentHeld = GrabComp;
		}
		else
		{
			Client_ItemPickupInvalid();
		}
	}
}

void AVRHand::Server_DroppedGrabbedComponent_Implementation()
{
	if (!_ComponentHeld)
	{
		//Client_ItemPickupInvalid();
		return;
	}

	_ComponentHeld->ReleaseComponent();
	_ComponentHeld = NULL;
}

void AVRHand::AttemptItemGrab()
{
	if (!_PhysicsHandSM || _ComponentHeld)
		return;

	UVRGrabComponent* GrabComp = FindClosestGrabComponent();

	if (GrabComp)
	{
		if (GrabComp->GrabComponent(this))
		{
			_ComponentHeld = GrabComp;
			_PhysicsHandCollision->SetSimulatePhysics(false);
			_PhysicsHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			if(GetLocalRole() < ENetRole::ROLE_Authority)
				Server_GrabbedComponent(_ComponentHeld);
		}
	}
}

UVRGrabComponent* AVRHand::FindClosestGrabComponent()
{
	if (!_PhysicsHandSM || !_GrabSphere || _GrabCompArray.Num() <= 0)
		return NULL;

	float ShortDist = 99999;
	UVRGrabComponent* GrabComp = NULL;

	for (UVRGrabComponent* GC : _GrabCompArray)
	{
		if (GC->CanGrabComponent())
		{
			float Dist = FVector::DistSquared(_GrabSphere->GetComponentLocation(), GC->GetComponentLocation());
			if (Dist < ShortDist)
			{
				ShortDist = Dist;
				GrabComp = GC;
				//GEngine->AddOnScreenDebugMessage(2, 1.0, FColor::Red, "Short Dist: " + FString::SanitizeFloat(ShortDist));
			}
		}
	}		

	return GrabComp;
}

void AVRHand::Server_NewHandTransform_Implementation(FVector PLocation, FRotator PRotation, FVector TLocation, FRotator TRotation)
{
	_LastKNownHandLocation = PLocation;
	_LastKNownHandRotation = PRotation;
	_LastKNownTrackingHandLocation = TLocation;
	_LastKNownTrackingHandRotation = TRotation;
}

void AVRHand::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRHand, _LastKNownHandLocation);
	DOREPLIFETIME(AVRHand, _LastKNownHandRotation);
	DOREPLIFETIME(AVRHand, _LastKNownTrackingHandLocation);
	DOREPLIFETIME(AVRHand, _LastKNownTrackingHandRotation);
}