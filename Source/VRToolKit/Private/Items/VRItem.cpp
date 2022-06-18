// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem.h"
#include "..\..\Public\Items\VRItem.h"
#include "Items/VRGrabComponent.h"
#include "Player/VRHand.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Online/NetworkHelpers.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "MotionControllerComponent.h"

// Sets default values
AVRItem::AVRItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	_ItemBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemBaseMesh");
	_ItemBaseMesh->SetSimulatePhysics(true);
	_ItemBaseMesh->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	SetRootComponent(_ItemBaseMesh);

	_OnCalculateCustomPhysics.BindUObject(this, &AVRItem::CustomPhysics);

	_MainGrabComponent = CreateDefaultSubobject<UVRGrabComponent>("MainGrabComponent");
	_MainGrabComponent->SetupAttachment(GetRootComponent());
	_MainGrabComponent->_ComponentGrabbed.AddDynamic(this, &AVRItem::MainGrabPointGrabbed);
	_MainGrabComponent->_ComponentReleased.AddDynamic(this, &AVRItem::MainGrabPointReleased);

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");
}

// Called when the game starts or when spawned
void AVRItem::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);

	_PHC->SetReplicatedPhysicsObject(_ItemBaseMesh);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	//_PHC->SetTargetObject(_MotionControllerComp);
	_PHC->SetMatchTarget(true);
}

// Called every frame
void AVRItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (!_MainGrabComponent->GetHand())
		return;
	
	if (GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		MoveItemUsingPhysics(DeltaTime);
		Server_SetNewTransform(_ItemBaseMesh->GetComponentLocation(), _ItemBaseMesh->GetComponentRotation());
	}
	else if (GetLocalRole() >= ENetRole::ROLE_SimulatedProxy)
	{
		FVector NewLocation;
		FVector CurrentLoc = _ItemBaseMesh->GetComponentLocation();
		NewLocation.X = FMath::FInterpTo(CurrentLoc.X, _LastKnownLocation.X, DeltaTime, 15.f);
		NewLocation.Y = FMath::FInterpTo(CurrentLoc.Y, _LastKnownLocation.Y, DeltaTime, 15.f);
		NewLocation.Z = FMath::FInterpTo(CurrentLoc.Z, _LastKnownLocation.Z, DeltaTime, 15.f);
		_ItemBaseMesh->SetWorldLocation(NewLocation);

		FRotator NewRotation;
		FRotator CurrentRot = _ItemBaseMesh->GetComponentRotation();
		NewRotation = FMath::RInterpTo(CurrentRot, _LastKnownRotation, DeltaTime, 10.f);
		_ItemBaseMesh->SetWorldRotation(NewRotation);
	}*/
}

float AVRItem::DistanceBetweenItemAndHand()
{
	if (!_MainGrabComponent->GetHand())
		return 1;

	return FVector::Dist(_ItemBaseMesh->GetComponentLocation(), _MainGrabComponent->GetHand()->GetTrackingHandTransform().GetLocation());
}

void AVRItem::ForceDrop(bool bDestroyAfter)
{
	if (!_MainGrabComponent || !_MainGrabComponent->GetHand())
		return;

	_MainGrabComponent->GetHand()->GripReleased();

	if (bDestroyAfter)
		Destroy();
}

bool AVRItem::IsBeingHeld()
{
	if (_MainGrabComponent && _MainGrabComponent->GetHand())
		return true;

	return false;
}

void AVRItem::PhysicsTick_Implementation(float SubsetDeltaTime)
{
	FBodyInstance* BM_BI = _ItemBaseMesh->GetBodyInstance();
	FTransform WorldT = BM_BI->GetUnrealWorldTransform_AssumesLocked();

	FTransform HandTransform = _MainGrabComponent->GetHand()->GetTrackingHandTransform();

	{
		//Linear Force
		FVector From = WorldT.GetLocation();
		FVector To = HandTransform.GetLocation();

		FVector Force = _LocPD.GetForce(SubsetDeltaTime, From, To);
		BM_BI->AddForce(Force * BM_BI->GetBodyMass(), false);
	}

	{
		FVector Torque = _RotPD.GetTorque(SubsetDeltaTime, WorldT.GetRotation(),
			HandTransform.GetRotation(), FMath::RadiansToDegrees(BM_BI->GetUnrealWorldAngularVelocityInRadians()),
			FVector(0, 0, 0), WorldT);

		BM_BI->AddTorqueInRadians(FMath::DegreesToRadians(Torque), false, true);
	}
}

void AVRItem::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	PhysicsTick_Implementation(DeltaTime);
}

void AVRItem::MainGrabPointGrabbed(AVRHand* Hand)
{
	if (!Hand || !Hand->GetPhysicsHandMesh())
		return;

	/*GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Main Grab Point Grabbed!", true);

	SetActorTickEnabled(true);

	const FPDController3D HandPD = Hand->GetPDController3D();
	_LocPD.Proportional = HandPD.Proportional;
	_LocPD.Derivative = HandPD.Derivative;

	const FQuatPDController HandQPD = Hand->GetQuatPDController();
	_RotPD.Frequency = HandQPD.Frequency;
	_RotPD.Dampening = HandQPD.Dampening;
	_RotPD.ForceMultiplier = HandQPD.ForceMultiplier;*/

	SetOwner(Hand->GetOwner());

	_PHC->SetTargetObject(Hand->GetMotionControllerComponent());
	_PHC->SetPlayerControllerOwner(Cast<APlayerController>(GetOwner()));
	_PHC->EnableComponent(true);

	if (GetLocalRole() >= ENetRole::ROLE_Authority)
	{
		NewMulticast_SomeonePickedUpItem(true);
	}
}

void AVRItem::MainGrabPointReleased()
{
	SetActorTickEnabled(false);

	_PHC->EnableComponent(false);
	_PHC->SetTargetObject(NULL);
	_PHC->SetPlayerControllerOwner(NULL);

	if (GetLocalRole() >= ENetRole::ROLE_Authority)
	{
		NewMulticast_SomeonePickedUpItem(false);
	}

	SetOwner(NULL);
}

void AVRItem::MoveItemUsingPhysics(float DeltaTime)
{
	/*{
		//Linear Force
		FVector From = _MainGrabComponent->GetComponentLocation();
		FVector To = _MainGrabComponent->GetHand()->GetTrackingHandTransform().GetLocation();

		FVector Force = _LocPD.GetForce(DeltaTime, From, To);
		_ItemBaseMesh->AddForce(Force * _ItemBaseMesh->GetMass());

		//Angular Force
		FQuat FQ = _MainGrabComponent->GetComponentQuat();
		FQuat TQ = _MainGrabComponent->GetHand()->GetTrackingHandTransform().GetRotation();
		FVector Vel = _ItemBaseMesh->GetPhysicsAngularVelocityInRadians();
		FVector IT = _ItemBaseMesh->GetInertiaTensor();

		FVector Torque = _RotPD.GetTorque(DeltaTime, FQ, TQ, Vel, IT, GetActorTransform());
		_ItemBaseMesh->AddTorqueInRadians(Torque /** _ItemBaseMesh->GetMass());
	}
	{
		/*FVector Axis = FVector::ZeroVector;
		float Angle = 0;
		FQuat Error = _MainGrabComponent->GetHand()->GetTrackingHandTransform().GetRotation() * _ItemBaseMesh->GetComponentQuat().Inverse();

		if (Error.W < 0)
		{
			Error.X = -Error.X;
			Error.Y = -Error.Y;
			Error.Z = -Error.Z;
			Error.W = -Error.W;
		}

		Error.ToAxisAndAngle(Axis, Angle);
		ExtraMaths::CorrectNormalizedVector(Axis);

		float AngleVel = _ItemBaseMesh->GetPhysicsAngularVelocityInDegrees().Size();
		FVector AxisAng = _ItemBaseMesh->GetPhysicsAngularVelocityInDegrees();
		ExtraMaths::CorrectNormalizedVector(AxisAng);
		FVector T = _TorsionSpringStiffness * Angle * Axis + _TorsionSpringDamping * AngleVel * -AxisAng;
		_ItemBaseMesh->AddTorqueInDegrees(T * _ItemBaseMesh->GetInertiaTensor());
	}*/

	if (_ItemBaseMesh->GetBodyInstance())
	{
		_ItemBaseMesh->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}
}

void AVRItem::NewMulticast_SomeonePickedUpItem_Implementation(bool bPickedUp)
{
	SetActorTickEnabled(bPickedUp);

	if (GetOwner() != UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (bPickedUp)
			_ItemBaseMesh->SetSimulatePhysics(false);
		else
			_ItemBaseMesh->SetSimulatePhysics(true);

	}
}

void AVRItem::Server_SetNewTransform_Implementation(FVector NewLoc, FRotator NewRot)
{
	_LastKnownLocation = NewLoc;
	_LastKnownRotation = NewRot;
}

void AVRItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRItem, _LastKnownLocation);
	DOREPLIFETIME(AVRItem, _LastKnownRotation);
}