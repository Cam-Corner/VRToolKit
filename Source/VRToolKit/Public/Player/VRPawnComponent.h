// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRPawnComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUVRPawnComponent, Log, All);

class UCapsuleComponent;
class UCameraComponent;
class USceneComponent;
class APawn;

UENUM(BlueprintType)
enum EMovementModes
{
	EMM_Grounded	UMETA("Grounded"),
	EMM_Falling		UMETA("Falling&Jumping"),
	EMM_Climbing	UMETA("Climbing"),
	EMM_Swimming	UMETA("Swimming"),
	EMM_Flying		UMETA("Flying"),
	EMM_Custom		UMETA("Custom"),
};

USTRUCT()
struct FVRPawnSnapShot
{
	GENERATED_BODY()

	FVector _Location;
	FRotator _Rotation;
};

USTRUCT()
struct FLastFloorHit
{
	GENERATED_BODY()

public:
	FVector _FloorHitDir = FVector::ZeroVector;
	FVector _FloorHitNormal = FVector::ZeroVector;
	float _FloorAngle = 0;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UVRPawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRPawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddMovementInput(FVector Direction, float Scale = 1);

	void AddRotationInput(FRotator Rot);

	void SetCachedComponents(UCapsuleComponent* Capsule, UCameraComponent* Camera, USceneComponent* VROrigin);

	void FollowCameraPitchRotation(bool bUsePitch);

	void FollowCameraYawRotation(bool bUseYaw);

	void FollowCameraRollRotation(bool bUseRoll);

	void RecenterHMD();

	void RecenterHMDClimbing();

	void HandGrabbedClimbingPoint(bool LeftHand, bool Grabbed);

	void DoJump();
private:
	const FVector ConsumeMovementInput();

	const FRotator ConsumeRotationInput();

	void ClientSideHandler(float DeltaTime);

	void SimulatedProxyHandler(float DeltaTime);

	void HandleGroundedMode(float DeltaTime);

	void HandleFallingMode(float DeltaTime);

	void HandleClimbingMode(float DeltaTime);

	void MoveCapsule(FVector Dir, float MoveAmount, float DeltaTime, bool bIgnoreWalkableSurface = false);

	void MoveCapsule(FVector FinalMove, bool bIgnoreWalkableSurface = false);

	bool IsWalkableSurface(const FHitResult& Hit);

	void HandleHMDInputRotation(float DeltaTime);

	void DoHMDInputRotation(float Amount);

	bool DoFloorCheck(FHitResult& FloorHit);

	FVector WorkOutCapsuleLocationForClimbing();

	FVector WorkOutTwoHandedCapsuleLocationForClimbing();

	FVector WorkOutOneHandedCapsuleLocationForClimbing(bool LeftHand);

	void ScaleCapsuleToPlayerHeight();

	void HandleJumping(float DeltaTime);

	bool SurfaceSteppable(float DeltaTime);
private:
	APawn* _OwningPawn = nullptr;

	UCapsuleComponent* _CachedCapsule = nullptr;

	UCameraComponent* _CachedCamera = nullptr;

	USceneComponent* _CachedVROrigin = nullptr;

//Variables
private:
	FVector _MovementDirToConsume;
	FRotator _RotationToConsume;

	FVector _LastRecievedLocation;

	bool _bUsingHMD = false;

	/*Current movement state*/
	TEnumAsByte<EMovementModes> _MovementState;

	FLastFloorHit _LastFloorHit;

	FVector _LastClimbingOffset = FVector::ZeroVector;

	bool _LeftHandGrabbedEGB = false;
	bool _RightHandGrabbedEGB = false;

	bool _InJump = false;
	float _JumpDisableTimer = 0;
	bool _CheckFloorStepFallingMode = false;

	FVector _Velocity = FVector::ZeroVector;

	float _GravityAcceleration = 0;

//server functions
private:
	UFUNCTION(Server, Reliable)
		void Server_SendMove(FVector NewLocation);

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_SendMove(FVector NewLocation);

//UPROPERTY Variables
protected:
	/*=================*/
	/*General Settings*/
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: General")
		bool _bUseVRMode = false;

	/*=================*/
	/* Camera Settings */
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		float _MaxCameraDistanceFromCollision = 25.0f;

	/* if true we use smooth turning else we use snap turning */
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		bool _bUseSmoothTurning = true;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		float _SmoothTurningSensitivity = 75.0f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		float _SnapTurningAmount = 45.0f;

	/*=================*/
	/*Grounded*/

	//Walking Speed
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		int _WalkSpeed = 250;

	//Running Speed
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		int _RunSpeed = 750;

	//max slope the player can walk up
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		int _MaxWalkableSlope = 55;

	//max stepup height
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _MaxStepUpHeight = 25.f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _StepUpVelocity = 25.f;

	/*=================*/
	/*Falling / Jumping*/
	//How Much Gravity should be applied when falling
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Falling/Jumping")
		float _Gravity = 750.0f;

	//How height can we jump
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Falling/Jumping")
		float _JumpZVel = 100;

	/*=================*/
	/*Climbing*/
	//Scale of the character collision box when climbing
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _ClimbingCapsuleHalfHeight = 22.f;

private:
	bool _bDoSnapTurning = false;

};
