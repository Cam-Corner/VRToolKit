// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utility/PIDControllers.h"
#include "GameFramework/Actor.h"
#include "VRHand.generated.h"


class UMotionControllerComponent;
class UStaticMeshComponent;
class UWidgetInteractionComponent;
class USceneComponent;
class UBoxComponent;
class UVRGrabComponent;
class UPhysicsConstraintComponent;
class AVRItem;
class UPhysicsReplicationComponent;
class UPhysicsHandlerComponent;
class USphereComponent;
class UEnvironmentGrabComponent;
class AVRCharacter;

USTRUCT()
struct FClimbingHandInfo
{
	GENERATED_BODY()

public:
	FVector _GrabbedLocation = FVector::ZeroVector;
	FVector _MC_CurrentLoc = FVector::ZeroVector;
	FVector _MC_MoveDiff = FVector::ZeroVector;

	FVector _EGC_StartLocation = FVector::ZeroVector;
	FVector _EGC_CurrentLoc = FVector::ZeroVector;
	FVector _EGC_MoveDiff = FVector::ZeroVector;

};

UCLASS()
class VRTOOLKIT_API AVRHand : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRHand();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void IsRightHand(bool bRightHand);

	bool GetIsRightHand() { return _bRightHand; }

	FVector GetMotionControllerMoveForwardDir();

	UStaticMeshComponent* GetPhysicsHandMesh() { return  _PhysicsHandSM; }

	void GripPressed();

	void GripReleased();

	void TopButtonPressed(bool bPressed);

	void BottomButtonPressed(bool bPressed);

	void TriggerPressed(float Value);

	const FPDController3D& GetPDController3D() { return _LocPD; }
	const FQuatPDController& GetQuatPDController() { return _RotPD; }

	const FTransform GetTrackingHandTransform();

	UMotionControllerComponent* GetMotionControllerComponent() { return _MotionControllerComp; }

	void UpdateOwner(AController* PC);

	FTransform GetPhysicsObjectTransform();

	FClimbingHandInfo GetClimbingHandInfo() { return _ClimbingInfo; }

	bool HandInClimbMode() { if (_GrabbedEGC) return true; return false; }

	void SetCharacterAttachedTo(AVRCharacter* Character) { _CharacterAttachedTo = Character; }

	UPrimitiveComponent* GetHandCollision() { return _PhysicsHandCollision; }

private:
	AVRCharacter* _CharacterAttachedTo;

	USceneComponent* _RootComp;

	UMotionControllerComponent* _MotionControllerComp;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _ControllerSM;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* _PhysicsHandCollision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _LeftPhysicsHandSM;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _RightPhysicsHandSM;

	UStaticMeshComponent* _PhysicsHandSM;

	void MovePhysicsHand(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
	FPDController3D _LocPD;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
	FQuatPDController _RotPD;

	UPROPERTY(Replicated)
	FVector _LastKNownHandLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FRotator _LastKNownHandRotation = FRotator::ZeroRotator;

	UPROPERTY(Replicated)
	FVector _LastKNownTrackingHandLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FRotator _LastKNownTrackingHandRotation = FRotator::ZeroRotator;

	float _ReplicatedHandTransformTimer = 0.25f;

	UFUNCTION(Server, UnReliable)
	void Server_NewHandTransform(FVector PLocation, FRotator PRotation, FVector TLocation, FRotator TRotation);

	UFUNCTION(Server, Reliable)
	void Server_GrabbedComponent(UVRGrabComponent* GrabComp);

	UFUNCTION(Server, Reliable)
	void Server_DroppedGrabbedComponent();

	UFUNCTION(Client, Reliable)
	void Client_ItemPickupInvalid();

	void AttemptItemGrab();

	UVRGrabComponent* FindClosestGrabComponent();

	UVRGrabComponent* _ComponentHeld = NULL;

	FCollisionResponseContainer _CorrectResponseContainer;

	virtual void PhysicsTick_Implementation(float SubsetDeltaTime);

	FCalculateCustomPhysics _OnCalculateCustomPhysics;

	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

	bool _bRightHand = false;
	
	void MoveHandOutsideOfHeldObject();

	UPhysicsReplicationComponent* _PRC;

	bool _bDoneGrab = false;

	UPROPERTY(EditAnywhere);
	UPhysicsHandlerComponent* _PHC;

	UPROPERTY(EditAnywhere);
	USphereComponent* _GrabSphere;

	TArray<UVRGrabComponent*> _GrabCompArray;

	TArray<UEnvironmentGrabComponent*> _EnvironmentGrabCompArray;

	UEnvironmentGrabComponent* _GrabbedEGC;

	FClimbingHandInfo _ClimbingInfo;

	void UpdateHandClimbInfo();
	FVector _LastHandLocation = FVector::ZeroVector;

	UFUNCTION()
		void GrabSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void GrabSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
