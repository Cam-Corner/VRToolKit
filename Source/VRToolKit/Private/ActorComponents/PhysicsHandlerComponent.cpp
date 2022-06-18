// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Online/NetworkHelpers.h"

// Sets default values for this component's properties
UPhysicsHandlerComponent::UPhysicsHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	_OnCalculateCustomPhysics.BindUObject(this, &UPhysicsHandlerComponent::CustomPhysics);
	SetIsReplicated(true);
	// ...

	_LocPD.Proportional = 5.0f;
	_LocPD.Derivative = 1.25f;
	_RotPD.Frequency = 150.0f;
	_RotPD.Dampening = 0.115f;
	_RotPD.ForceMultiplier = 250.f;
}


// Called when the game starts
void UPhysicsHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
	_DefaultTickTimer = 1 / NETWORKHELPERS_SERVER_TICKRATE;
	_TickTimer = _DefaultTickTimer;
	// ...
	
}

// Called every frame
void UPhysicsHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!_PhysicsObject || !_bComponentEnabled)
		return;

	if (_PhysicsObject->GetBodyInstance())
	{
		_PhysicsObject->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}
	// ...
}

void UPhysicsHandlerComponent::SetMatchTargetAuthorityType(EAuthorityType MatchTargetAuthorityType)
{
	_MatchTargetAuthorityType = MatchTargetAuthorityType;
}

void UPhysicsHandlerComponent::SetTargetObject(USceneComponent* Comp)
{
	_TargetObject = Comp;
}

void UPhysicsHandlerComponent::SetMatchTarget(bool bMatchTarget)
{
	_bMatchTarget = bMatchTarget;
}

void UPhysicsHandlerComponent::SetPlayerControllerOwner(AController* PC)
{
	_PCOwner = PC;
}

void UPhysicsHandlerComponent::EnableComponent(bool bEnabled)
{
	_bComponentEnabled = bEnabled;
}

void UPhysicsHandlerComponent::SetTargetLocationOffset(FVector Offset)
{
	_TargetLocationOffset = Offset;
}

void UPhysicsHandlerComponent::SetTargetRotationOffset(FQuat Offset)
{
	_TargetRotationOffset = Offset;
}

void UPhysicsHandlerComponent::UpdateComponent(float DeltaTime)
{
	if (!_PhysicsObject || !_bComponentEnabled)
		return;

	if (_PhysicsObject->GetBodyInstance())
	{
		_PhysicsObject->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}
}


void UPhysicsHandlerComponent::UpdateToNextPhysicsFrame()
{
	if (_TickTimer > 0 || !_PhysicsObject)
		return;

	FBodyInstance* PO_BI = _PhysicsObject->GetBodyInstance();

	if (!PO_BI)
		return;

	FPhysicsFrame PF = GetNewPhysicsFrame();

	FTransform T;
	T.SetLocation(PF._Location);
	T.SetRotation(PF._Rotation);
	PO_BI->SetBodyTransform(T, ETeleportType::ResetPhysics);

	PO_BI->SetLinearVelocity(PF._Linear_Velocity, false);
	PO_BI->SetAngularVelocityInRadians(PF._Angular_Velocity, false);

	_Known_Goto_Location = PF._Goto_Location;
	_Known_Goto_Rotation = PF._Goto_Rotation;
}


void UPhysicsHandlerComponent::PhysicsTick_DefaultPhysics(float SubsetDeltaTime)
{
	if (_PCOwner)
	{
		if (_PCOwner->GetLocalRole() >= ENetRole::ROLE_Authority)
		{
			FBodyInstance* PO_BI = _PhysicsObject->GetBodyInstance();

			if (!PO_BI)
				return;

			if (_TickTimer <= 0)
			{
				FTransform WorldT = PO_BI->GetUnrealWorldTransform_AssumesLocked();

				FPhysicsFrame PF;
				PF._Location = WorldT.GetLocation();
				PF._Rotation = WorldT.GetRotation();
				PF._Linear_Velocity = PO_BI->GetUnrealWorldVelocity();
				PF._Angular_Velocity = PO_BI->GetUnrealWorldAngularVelocityInRadians();

				//If both velocitys are 0 then their is no need to send a physics update (helps to save bandwith when the object isnt moving)
				if (PF._Linear_Velocity != FVector::ZeroVector || PF._Angular_Velocity != FVector::ZeroVector)
					SendPhysicsTick(PF);
			}
		}
		else
		{
			UpdateToNextPhysicsFrame();
		}
	}
	else
	{
		UpdateToNextPhysicsFrame();
	}
}

void UPhysicsHandlerComponent::PhysicsTick_MatchTarget(float SubsetDeltaTime)
{
	if (!_PhysicsObject || !_TargetObject)
		return;

	FBodyInstance* PO_BI = _PhysicsObject->GetBodyInstance();

	EAuthorityType EAT = _MatchTargetAuthorityType;

	if (!_PCOwner || !PO_BI)
		return;

	ENetRole ENR = _PCOwner->GetLocalRole();
	
	FTransform WorldT = PO_BI->GetUnrealWorldTransform_AssumesLocked();

	FTransform TTransform;

	
	if ((EAT == EAuthorityType::EAT_Client && _PCOwner == UGameplayStatics::GetPlayerController(GetWorld(), 0)) 
		|| (EAT != EAuthorityType::EAT_Client && ENR >= ENetRole::ROLE_Authority))
	{
		TTransform = _TargetObject->GetComponentTransform();
		
		if (_TickTimer <= 0)
		{
			FPhysicsFrame PF;
			PF._Location = WorldT.GetLocation();
			PF._Rotation = WorldT.GetRotation();
			PF._Linear_Velocity = PO_BI->GetUnrealWorldVelocity();
			PF._Angular_Velocity = PO_BI->GetUnrealWorldAngularVelocityInRadians();
			PF._Goto_Location = TTransform.GetLocation();
			PF._Goto_Rotation = TTransform.GetRotation();

			//If both velocitys are 0 then their is no need to send a physics update (helps to save bandwith when the object isnt moving)
			if (PF._Linear_Velocity != FVector::ZeroVector || PF._Angular_Velocity != FVector::ZeroVector)
				SendPhysicsTick(PF);
		}
	}
	else 
	{
		UpdateToNextPhysicsFrame();

		TTransform.SetLocation(_Known_Goto_Location);
		TTransform.SetRotation(_Known_Goto_Rotation);
	}

	{
		//Linear Force
		FVector From = WorldT.GetLocation();
		FVector To = TTransform.GetLocation() + _TargetLocationOffset;

		FVector Force = _LocPD.GetForce(SubsetDeltaTime, From, To);
		PO_BI->AddForce(Force * PO_BI->GetBodyMass(), false);
	}

	{
		FQuat From = WorldT.GetRotation();
		FQuat To = TTransform.GetRotation(); //*;
		//FQuat Offset = FQuat(FVector::CrossProduct(To.GetForwardVector(), To.GetUpVector()), -45);
		To *= _TargetRotationOffset;

		FVector Torque = _RotPD.GetTorque(SubsetDeltaTime, From,
			To, FMath::RadiansToDegrees(PO_BI->GetUnrealWorldAngularVelocityInRadians()),
			FVector(0, 0, 0), WorldT);

		PO_BI->AddTorqueInRadians(FMath::DegreesToRadians(Torque), false, true);
	}
}

void UPhysicsHandlerComponent::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	_TickTimer -= DeltaTime;

	if (!_bMatchTarget)
		PhysicsTick_DefaultPhysics(DeltaTime);
	else
		PhysicsTick_MatchTarget(DeltaTime);


	if (_TickTimer <= 0)
		_TickTimer = _DefaultTickTimer;
}
