// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRPawnComponent.h"
#include "..\..\Public\Player\VRPawnComponent.h"
#include "Utility/ExtraMaths.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Player/VRHand.h"
#include "Player/VRCharacter.h"

DEFINE_LOG_CATEGORY(LogUVRPawnComponent);

// Sets default values for this component's properties
UVRPawnComponent::UVRPawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UVRPawnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	if (APawn* Owner = Cast<APawn>(GetOwner()))
	{
		_OwningPawn = Owner;
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	}

	if (_CachedCapsule)
		_LastRecievedLocation = _CachedCapsule->GetComponentLocation();

	RecenterHMD();
}


// Called every frame
void UVRPawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	ENetRole Role = GetOwnerRole();

	if (_OwningPawn && _OwningPawn->IsLocallyControlled())
	{
		ClientSideHandler(DeltaTime);

		/*For cases where we havnt consumed the input / rotation vectors, we should do it anyway so it doesnt happen in a different frame
		and be unexpected for the player*/
		/*FVector ConsumedMove = ConsumeMovementInput();
		FRotator ConsumedRot = ConsumeRotationInput();*/
	}
	else if (Role >= ROLE_SimulatedProxy)
	{
		SimulatedProxyHandler(DeltaTime);
	}
}

void UVRPawnComponent::AddMovementInput(FVector Direction, float Scale)
{
	_MovementDirToConsume += Direction * Scale;
}

void UVRPawnComponent::AddRotationInput(FRotator Rot)
{
	_RotationToConsume += Rot;
}

void UVRPawnComponent::SetCachedComponents(UCapsuleComponent* Capsule, UCameraComponent* Camera, USceneComponent* VROrigin)
{
	_CachedCapsule = Capsule;
	_CachedCamera = Camera;
	_CachedVROrigin = VROrigin;
}

void UVRPawnComponent::FollowCameraPitchRotation(bool bUsePitch)
{

}

void UVRPawnComponent::FollowCameraYawRotation(bool bUseYaw)
{

}

void UVRPawnComponent::FollowCameraRollRotation(bool bUseRoll)
{

}

void UVRPawnComponent::RecenterHMD()
{
	if (!_CachedCamera || !_CachedCapsule || !_CachedVROrigin)
		return;

	FVector MoveOffset;
	float HalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();

	MoveOffset.X = _CachedCapsule->GetComponentLocation().X - _CachedCamera->GetComponentLocation().X;
	MoveOffset.Y = _CachedCapsule->GetComponentLocation().Y - _CachedCamera->GetComponentLocation().Y;
	MoveOffset.Z = (_CachedCapsule->GetComponentLocation().Z - HalfHeight) - _CachedVROrigin->GetComponentLocation().Z;

	FVector NewLocation = /*_CachedVROrigin->GetComponentLocation() +*/ MoveOffset;
	_CachedVROrigin->AddWorldOffset(MoveOffset);
	_CachedCapsule->AddWorldOffset(-MoveOffset);
}

void UVRPawnComponent::RecenterHMDClimbing()
{
	if (!_CachedCamera || !_CachedCapsule || !_CachedVROrigin)
		return;

	FVector MoveOffset;
	float HalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();

	MoveOffset.X = _CachedCapsule->GetComponentLocation().X - _CachedCamera->GetComponentLocation().X;
	MoveOffset.Y = _CachedCapsule->GetComponentLocation().Y - _CachedCamera->GetComponentLocation().Y;
	MoveOffset.Z = (_CachedCapsule->GetComponentLocation().Z + HalfHeight) - _CachedCamera->GetComponentLocation().Z;

	FVector NewLocation = /*_CachedVROrigin->GetComponentLocation() +*/ MoveOffset;
	_CachedVROrigin->AddWorldOffset(MoveOffset);
	_CachedCapsule->AddWorldOffset(-MoveOffset);
}

void UVRPawnComponent::HandGrabbedClimbingPoint(bool LeftHand, bool Grabbed)
{
	if (LeftHand)
		_LeftHandGrabbedEGB = Grabbed;
	else
		_RightHandGrabbedEGB = Grabbed;

	if (_LeftHandGrabbedEGB || _RightHandGrabbedEGB)
	{
		_MovementState = EMovementModes::EMM_Climbing;
		
		if (_CachedCapsule && _CachedCamera)
		{
			_CachedCapsule->SetCapsuleHalfHeight(_ClimbingCapsuleHalfHeight);
			_CachedCapsule->SetWorldLocation(_CachedCamera->GetComponentLocation() 
				- FVector(0, 0, _CachedCapsule->GetScaledCapsuleHalfHeight()));
		}
	}
	else
		_MovementState = EMovementModes::EMM_Falling;


}

const FVector UVRPawnComponent::ConsumeMovementInput()
{
	if (_MovementDirToConsume.Size() > 1)
	{
		ExtraMaths::CorrectNormalizedVector(_MovementDirToConsume);
	}

	FVector ReturnVec = _MovementDirToConsume;
	_MovementDirToConsume = FVector::ZeroVector;
	return ReturnVec;
}

const FRotator UVRPawnComponent::ConsumeRotationInput()
{
	FRotator ReturnRot = _RotationToConsume;
	_RotationToConsume = FRotator::ZeroRotator;
	return ReturnRot;
}

void UVRPawnComponent::ClientSideHandler(float DeltaTime)
{
	if (!_CachedCamera || !_CachedCapsule)
		return;

	_bUsingHMD = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
	

	if (!_bUsingHMD)
	{
		FRotator RotOffset = ConsumeRotationInput();
		_CachedCamera->SetWorldLocation(_CachedCapsule->GetComponentLocation());
		_CachedCamera->AddWorldOffset(FVector(0, 0, _CachedCapsule->GetScaledCapsuleHalfHeight()));

		FRotator CapRot = RotOffset;
		CapRot.Pitch = 0;
		_CachedCapsule->AddLocalRotation(CapRot * 250 * DeltaTime);

		FRotator OldRot = _CachedCamera->GetComponentRotation();
		_CachedCamera->SetWorldRotation(FRotator::ZeroRotator);
		float YawRot = OldRot.Yaw + (RotOffset.Yaw * 250 * DeltaTime);
		float PitchRot = OldRot.Pitch + (RotOffset.Pitch * 250 * DeltaTime);

		if (PitchRot > 80)
		{
			PitchRot = 80;
		}

		if (PitchRot < -80)
		{
			PitchRot = -80;
		}

		_CachedCamera->AddLocalRotation(FRotator(0, YawRot, 0));
		_CachedCamera->AddLocalRotation(FRotator(PitchRot, 0, 0));
	}
	else
	{
		HandleHMDInputRotation(DeltaTime);
	}

	//Handle the correct movement mode
	switch (_MovementState)
	{
	case EMovementModes::EMM_Grounded:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Grounded");
		HandleGroundedMode(DeltaTime);
		break;
	case EMovementModes::EMM_Falling:
		GEngine->AddOnScreenDebugMessage(99, .5f, FColor::Yellow, "Movement State: Falling");
		HandleFallingMode(DeltaTime);
		break;
	case EMovementModes::EMM_Climbing:
		HandleClimbingMode(DeltaTime);
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Climbing");
		break;
	case EMovementModes::EMM_Swimming:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Swimming");
		break;
	case EMovementModes::EMM_Flying:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Flying");
		break;
	case EMovementModes::EMM_Custom:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Custom");
		break;
	default:
		break;
	}


	//Send info to server
	FVRPawnSnapShot PSS;
	PSS._Location = _CachedCapsule->GetComponentLocation();
	PSS._Rotation = _CachedCapsule->GetComponentRotation();
	Server_SendMove(PSS._Location);

}

void UVRPawnComponent::SimulatedProxyHandler(float DeltaTime)
{
	if (!_CachedCapsule)
		return;

	//_CachedCapsule->SetWorldLocation(_LastRecievedLocation);
	
	FVector CurrentLoc = _CachedCapsule->GetComponentLocation();
	float InterpSpeed = 15.0f;
	FVector NewLoc;
	NewLoc.X = FMath::FInterpTo(CurrentLoc.X, _LastRecievedLocation.X, DeltaTime, InterpSpeed);
	NewLoc.Y = FMath::FInterpTo(CurrentLoc.Y, _LastRecievedLocation.Y, DeltaTime, InterpSpeed);
	NewLoc.Z = FMath::FInterpTo(CurrentLoc.Z, _LastRecievedLocation.Z, DeltaTime, InterpSpeed);
	_CachedCapsule->SetWorldLocation(NewLoc);

	/*DrawDebugCapsule(GetWorld(), _LastRecievedLocation, _CachedCapsule->GetScaledCapsuleHalfHeight(),
		_CachedCapsule->GetScaledCapsuleRadius(), _CachedCapsule->GetComponentQuat(), FColor::Red, false, 0.01f, 
		(uint8)'\000', 2.5f);*/
}

void UVRPawnComponent::HandleGroundedMode(float DeltaTime)
{
	ScaleCapsuleToPlayerHeight();

	//Do a floor check
	FHitResult FloorCheckHit;
	bool HitFloor = DoFloorCheck(FloorCheckHit);

	if (!HitFloor || _InJump)
	{
		if (_InJump || !SurfaceSteppable(DeltaTime))
		{
			_MovementState = EMovementModes::EMM_Falling;
			return;
		}
	}

	/* Make capsule face the direction of the movement forward dir (For example the HMD) */
	FRotator Rot = _CachedCapsule->GetComponentRotation();
	Rot.Yaw = _CachedCamera->GetComponentRotation().Yaw;
	_CachedCapsule->SetWorldRotation(Rot);
	
	//DrawDebugLine(GetWorld(), FloorCheckHit.ImpactPoint, FloorCheckHit.ImpactPoint + (FloorCheckHit.ImpactNormal * 50), FColor::Red, false, 5.f);

	//Check if floor is walkable
	if (IsWalkableSurface(FloorCheckHit))
	{
		GEngine->AddOnScreenDebugMessage(112, 1.f, FColor::Yellow, "Movement State: Grounded: Walkable");
		FVector MoveDir = ConsumeMovementInput();

		if (MoveDir != FVector::ZeroVector)
		{
			GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Grounded: Moving");
			//MoveDir = FVector::VectorPlaneProject(MoveDir, _LastFloorHit._FloorHitNormal);
			//MoveCapsule(MoveDir, _RunSpeed, DeltaTime, true);
			_Velocity = MoveDir * _RunSpeed;
			
			if (_bUsingHMD)
			{
				RecenterHMD();	
			}
		}
		else
		{
			_Velocity.X = 0;
			_Velocity.Y = 0;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(111, 1.f, FColor::Yellow, "Movement State: Grounded: Sliding");
		FVector MoveDir = _LastFloorHit._FloorHitNormal;
		MoveDir.Z = 0;
		MoveDir.Normalize();
		MoveDir = FVector::VectorPlaneProject(MoveDir, _LastFloorHit._FloorHitNormal);

		float DotP = FVector::DotProduct(FVector(0, 0, 1), _LastFloorHit._FloorHitNormal);
		float SlideSpeed = (_RunSpeed * (2 - DotP)) / 2;
		GEngine->AddOnScreenDebugMessage(98, .1f, FColor::Yellow, "Slide Speed: " + FString::SanitizeFloat(SlideSpeed));
		//DrawDebugLine(GetWorld(), FloorCheckHit.ImpactPoint, FloorCheckHit.ImpactPoint + (MoveDir * 50), FColor::Green, false, 5.f);

		//MoveCapsule(MoveDir, SlideSpeed, DeltaTime, true);
		_Velocity = MoveDir * SlideSpeed;
		if (_bUsingHMD)
		{
			RecenterHMD();
		}
	}

	FVector Dir = _Velocity;
	Dir.Normalize();
	float Amount = _Velocity.Size();

	MoveCapsule(Dir, Amount, DeltaTime, false);
}

void UVRPawnComponent::HandleFallingMode(float DeltaTime)
{
	if (_JumpDisableTimer <= 0)
		_InJump = false;
	else
		_JumpDisableTimer -= DeltaTime;

	GEngine->AddOnScreenDebugMessage(53, 5.0f, FColor::Red, "ShouldGoIntoJump");

	_Velocity.Z -= _Gravity * DeltaTime;

	RecenterHMD();
	//ScaleCapsuleToPlayerHeight();
	
	if (!_InJump)
	{
		FHitResult FloorHit;
		bool FloorCheck = DoFloorCheck(FloorHit);

		if (FloorCheck)
		{
			_MovementState = EMovementModes::EMM_Grounded;
			_Velocity.Z = 0;
		}
	}

	FVector Dir = _Velocity;
	Dir.Normalize();
	float Amount = _Velocity.Size();
	MoveCapsule(Dir, Amount, DeltaTime, false);
}

void UVRPawnComponent::HandleClimbingMode(float DeltaTime)
{
	if (!_CachedCapsule || !_CachedVROrigin)
		return;

	FVector GotoLoc = WorkOutCapsuleLocationForClimbing();
	
	FVector OldOrigin = _CachedVROrigin->GetComponentLocation();

	FVector Offset = GotoLoc;// -_CachedCapsule->GetComponentLocation();
	//MoveCapsule(Offset, true);
	_CachedCapsule->AddWorldOffset(Offset, true);
	RecenterHMDClimbing();

	_LastClimbingOffset = _CachedVROrigin->GetComponentLocation() - OldOrigin;

}

void UVRPawnComponent::MoveCapsule(FVector Dir, float MoveAmount, float DeltaTime, bool bIgnoreWalkableSurface)
{
	if (!_CachedCapsule)
		return;

	/* Move the character capsule in a direction */
	FHitResult MovementHit;
	FVector Offset = (Dir * MoveAmount) * DeltaTime;
	_CachedCapsule->AddWorldOffset(Offset, true, &MovementHit);

	if (MovementHit.bBlockingHit)
	{
		if (bIgnoreWalkableSurface || IsWalkableSurface(MovementHit))
		{
			FVector PlaneProject = FVector::VectorPlaneProject(Offset, MovementHit.ImpactNormal);
			ExtraMaths::CorrectNormalizedVector(PlaneProject);
			Offset = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(Offset, true);
		}
		else
		{
			SurfaceSteppable(DeltaTime);
			
			FVector Normal = MovementHit.ImpactNormal;
			Normal.Z = 0;
			Normal.Normalize();
			//ExtraMaths::CorrectNormalizedVector(Normal);

			FVector PlaneProject = FVector::VectorPlaneProject(Offset, Normal);
			Offset = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(Offset, true);
		}
	}
}

void UVRPawnComponent::MoveCapsule(FVector FinalMove, bool bIgnoreWalkableSurface)
{
	if (!_CachedCapsule)
		return;

	/* Move the character capsule in a direction */
	FHitResult MovementHit;
	_CachedCapsule->AddWorldOffset(FinalMove, true, &MovementHit);

	if (MovementHit.bBlockingHit)
	{
		if (bIgnoreWalkableSurface || IsWalkableSurface(MovementHit))
		{
			FVector PlaneProject = FVector::VectorPlaneProject(FinalMove, MovementHit.Normal);
			ExtraMaths::CorrectNormalizedVector(PlaneProject);
			FinalMove = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(FinalMove, true);
		}
		else
		{
			FVector Normal = MovementHit.Normal;
			Normal.Z = 0;
			Normal.Normalize();
			//ExtraMaths::CorrectNormalizedVector(Normal);

			FVector PlaneProject = FVector::VectorPlaneProject(FinalMove, Normal);
			FinalMove = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(FinalMove, true);
		}
	}
}

bool UVRPawnComponent::IsWalkableSurface(const FHitResult& Hit)
{
	if (!Hit.bBlockingHit || Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
		return false;
//	DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + (Hit.ImpactNormal * 50), FColor::Blue, false, 0.1f);
//	DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + (Hit.Normal * 50), FColor::Red, false, 0.1f);
	float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), Hit.ImpactNormal);
	if (Angle > _MaxWalkableSlope)
	{
		GEngine->AddOnScreenDebugMessage(34, .1f, FColor::Red, "Angle: " + FString::SanitizeFloat(Angle) + ": NotWalkable", false);
		return false;
	}

	GEngine->AddOnScreenDebugMessage(34, .1f, FColor::Red, "Angle: " + FString::SanitizeFloat(Angle) + ": IsWalkable", false);

	return true;
}

void UVRPawnComponent::HandleHMDInputRotation(float DeltaTime)
{
	FRotator RotOffset = ConsumeRotationInput();

	if (!_bDoSnapTurning && (RotOffset.Yaw > -0.3f && RotOffset.Yaw < 0.3f))
		_bDoSnapTurning = true;

	float FinalAmount = 0;
	_bUseSmoothTurning ? FinalAmount = _SmoothTurningSensitivity * RotOffset.Yaw * DeltaTime
		: FinalAmount = _SnapTurningAmount * (RotOffset.Yaw > 0 ? 1 : -1);

	if (_bUseSmoothTurning || (_bDoSnapTurning && (RotOffset.Yaw > 0.5f || RotOffset.Yaw < -0.5f)))
	{
		DoHMDInputRotation(FinalAmount);
		_bDoSnapTurning = false;
	}
}

void UVRPawnComponent::DoHMDInputRotation(float Amount)
{
	if (!_CachedVROrigin || !_CachedCamera || !_CachedCapsule)
		return;

	//New Rotation
	FVector Distance = _CachedVROrigin->GetComponentLocation() - _CachedCamera->GetComponentLocation();
	FVector Rotation = Distance.RotateAngleAxis(Amount, FVector(0, 0, 1));
	FVector FinalLocation = _CachedCamera->GetComponentLocation() + Rotation;

	_CachedVROrigin->SetWorldLocation(FinalLocation);
	_CachedVROrigin->AddRelativeRotation(FRotator(0, Amount, 0));
	
}

bool UVRPawnComponent::DoFloorCheck(FHitResult& FloorHit)
{
	FVector FloorCheckStart = _CachedCapsule->GetComponentLocation();
	FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, 1.f);
	FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
	FCollisionQueryParams ColParams;
	ColParams.AddIgnoredComponent(_CachedCapsule);

	bool ReturnVal = GetWorld()->SweepSingleByChannel(FloorHit, FloorCheckStart, FloorCheckEnd, _CachedCapsule->GetComponentQuat(),
		ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

	DrawDebugCapsule(GetWorld(), FloorCheckEnd, FloorCheckShape.GetCapsuleHalfHeight(), FloorCheckShape.GetCapsuleRadius(),
		 _CachedCapsule->GetComponentQuat(), FColor::Red, false, 0.01f);

	FVector MoveDir = FVector::VectorPlaneProject(FVector(0, 0, -1), FloorHit.ImpactNormal);
	_LastFloorHit._FloorHitDir = MoveDir;
	_LastFloorHit._FloorAngle = ExtraMaths::GetAngleOfTwoVectors(FloorHit.ImpactNormal, FVector(0, 0, 1));
	_LastFloorHit._FloorHitNormal = FloorHit.ImpactNormal;

	return ReturnVal;
}

void UVRPawnComponent::DoJump()
{
	GEngine->AddOnScreenDebugMessage(51, 5.0f, FColor::Red, "DoJumpComp");

	if (_InJump || !_CachedCapsule || _MovementState == EMovementModes::EMM_Falling)
		return;

	_Velocity.Z = _JumpZVel;
	_InJump = true;
	_JumpDisableTimer = 1.f;
}

FVector UVRPawnComponent::WorkOutCapsuleLocationForClimbing()
{
	FVector NewCapLoc = FVector::ZeroVector;

	if (_LeftHandGrabbedEGB && _RightHandGrabbedEGB)
		NewCapLoc = WorkOutTwoHandedCapsuleLocationForClimbing();
	else if (_LeftHandGrabbedEGB)
		NewCapLoc = WorkOutOneHandedCapsuleLocationForClimbing(true);
	else if (_RightHandGrabbedEGB)
		NewCapLoc = WorkOutOneHandedCapsuleLocationForClimbing(false);

	return NewCapLoc;
}

FVector UVRPawnComponent::WorkOutTwoHandedCapsuleLocationForClimbing()
{
	FClimbingHandInfo LeftHandClimbInfo;
	FClimbingHandInfo RightHandClimbInfo;

	if (AVRCharacter* VRC = Cast<AVRCharacter>(_OwningPawn))
	{
		LeftHandClimbInfo = VRC->GetLeftHandClimbInfo();
		RightHandClimbInfo = VRC->GetRightHandClimbInfo();

		FVector FinalLefttLoc = LeftHandClimbInfo._GrabbedLocation + LeftHandClimbInfo._EGC_MoveDiff;
		FVector FinalRightLoc = RightHandClimbInfo._GrabbedLocation + RightHandClimbInfo._EGC_MoveDiff;
		

		//Find Center Point of grabbed location
		FVector GrabbedCentrePoint = FinalRightLoc - FinalLefttLoc;
		GrabbedCentrePoint /= 2;
		GrabbedCentrePoint += FinalLefttLoc;

		//find center point of current hand locations
		FVector CurrentCentrePoint = RightHandClimbInfo._MC_CurrentLoc - LeftHandClimbInfo._MC_CurrentLoc;
		CurrentCentrePoint /= 2;
		CurrentCentrePoint += LeftHandClimbInfo._MC_CurrentLoc;

		// find difference for the offset
		FVector Offset = CurrentCentrePoint - GrabbedCentrePoint;
		return -Offset;
	}

	return FVector::ZeroVector;
}

FVector UVRPawnComponent::WorkOutOneHandedCapsuleLocationForClimbing(bool LeftHand)
{
	FClimbingHandInfo HandClimbInfo;

	if (AVRCharacter* VRC = Cast<AVRCharacter>(_OwningPawn))
	{
		if (LeftHand)
			HandClimbInfo = VRC->GetLeftHandClimbInfo();
		else
			HandClimbInfo = VRC->GetRightHandClimbInfo();

		return -HandClimbInfo._MC_MoveDiff;
	}

	return FVector::ZeroVector;
}

void UVRPawnComponent::ScaleCapsuleToPlayerHeight()
{
	return;

	if (!_CachedVROrigin || !_CachedCamera || !_CachedCapsule)
		return;

	float NewHalfHeight = _CachedCamera->GetComponentLocation().Z - _CachedVROrigin->GetComponentLocation().Z;
	NewHalfHeight /= 2;

	if (NewHalfHeight < _CachedCapsule->GetScaledCapsuleHalfHeight())
	{
		_CachedCapsule->SetCapsuleHalfHeight(NewHalfHeight);
		_CachedCapsule->AddWorldOffset(FVector(0, 0, -NewHalfHeight), true);
	}
	else
	{
		_CachedCapsule->SetCapsuleHalfHeight(NewHalfHeight);
		_CachedCapsule->AddWorldOffset(FVector(0, 0, NewHalfHeight * 1.5f));
		_CachedCapsule->AddWorldOffset(FVector(0, 0, -NewHalfHeight * 2), true);
	}
}

void UVRPawnComponent::HandleJumping(float DeltaTime)
{
	/*if (!_InJump || !_CachedCapsule || _CachedCapsule->GetComponentLocation().Z > _JumpEndZ)
		return;

	float JumpOffset = 0;

	if (_CachedCapsule->GetComponentLocation().Z + _JumpAcceleration > _JumpEndZ)
	{		
		_InJump = false;
		_Velocity.Z = _JumpAcceleration;
	}
	else
	{
		JumpOffset = _JumpAcceleration;
	}

	FHitResult Hit;
	FVector Offset = FVector(_LastMoveOffset.X, _LastMoveOffset.Y, JumpOffset);
	FVector Dir = Offset;
	Dir.Normalize();
	float Amount = Offset.Size();
	//MoveCapsule(Dir, Amount, DeltaTime, true);
	_Velocity.Z += _JumpAcceleration;
	GEngine->AddOnScreenDebugMessage(53, 5.0f, FColor::Red, "Jumped");

	if (Hit.bBlockingHit)
	{
		_InJump = false;
		_Velocity.Z = 0;
	}*/
}

bool UVRPawnComponent::SurfaceSteppable(float DeltaTime)
{
	if (!_CachedCapsule)
		return false;

	FHitResult Hit;
	FCollisionShape Shape = _CachedCapsule->GetCollisionShape();

	FVector GoingDir = _Velocity * DeltaTime;
	GoingDir.Z += _MaxStepUpHeight;
	FVector StartLoc = _CachedCapsule->GetComponentLocation() + GoingDir;
	FVector EndLoc = StartLoc - FVector(0, 0, _MaxStepUpHeight * 2);
	FCollisionQueryParams Params;
	Params.AddIgnoredComponent(_CachedCapsule);

	GetWorld()->SweepSingleByChannel(Hit, StartLoc, EndLoc, _CachedCapsule->GetComponentQuat(), ECC_Pawn, Shape, Params);
	
	/*if (Hit.bBlockingHit)
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint + FVector(0, 0, _CachedCapsule->GetScaledCapsuleRadius()), _CachedCapsule->GetScaledCapsuleRadius(), 32, FColor::Red, false, 2.5f);
	else
		DrawDebugSphere(GetWorld(), EndLoc, _CachedCapsule->GetScaledCapsuleRadius(), 32, FColor::Blue, false, 2.5f);*/

	if (!Hit.bBlockingHit)
		return false;

	if (Hit.Location.Z > _CachedCapsule->GetComponentLocation().Z && !IsWalkableSurface(Hit))
		return false;

	if (Hit.Location.Z != _CachedCapsule->GetComponentLocation().Z)
	{
		FVector CurrentLoc = _CachedCapsule->GetComponentLocation();
		CurrentLoc.Z = Hit.Location.Z + 0.5f;
		_CachedCapsule->SetWorldLocation(CurrentLoc, true);
		_CachedCapsule->AddWorldOffset(_Velocity * DeltaTime, true);
	}
	return true;
}

void UVRPawnComponent::NetMulticast_SendMove_Implementation(FVector NewLocation)
{
	_LastRecievedLocation = NewLocation;
}

void UVRPawnComponent::Server_SendMove_Implementation(FVector NewLocation)
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, "NewMove");
		GEngine->AddOnScreenDebugMessage(2, 1.5f, FColor::Green, "New Location: " + NewLocation.ToString());
	}*/

	NetMulticast_SendMove(NewLocation);
}

