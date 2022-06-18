// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utility/PIDControllers.h"
#include "GameFramework/Actor.h"
#include "VRItem.generated.h"

class UVRGrabComponent;
class AVRHand;
class UStaticMeshComponent;
class UPhysicsHandlerComponent;

UCLASS()
class VRTOOLKIT_API AVRItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float DistanceBetweenItemAndHand();

	void ForceDrop(bool bDestroyAfter = false);

	bool IsBeingHeld();
protected:
	UFUNCTION()
	virtual void MainGrabPointGrabbed(AVRHand* Hand);

	UFUNCTION()
	virtual void MainGrabPointReleased();

	virtual void MoveItemUsingPhysics(float DeltaTime);

	UPROPERTY(VisibleAnywhere)
	UVRGrabComponent* _MainGrabComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _ItemBaseMesh;

	UPROPERTY(EditAnywhere, Category = "Spring")
	FPDController3D _LocPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
	FQuatPDController _RotPD;
	//virtual void TriggerPressed();
	//
	//virtual void BottomButtonPressed();
	//
	//virtual void TopButtonPressed();

//	void SetPhysicsMoveToPoint(FVector MoveTo);

//networked
protected:
	UPROPERTY(Replicated)
	FVector _LastKnownLocation;

	UPROPERTY(Replicated)
	FRotator _LastKnownRotation;

	UFUNCTION(Server, UnReliable)
	void Server_SetNewTransform(FVector NewLoc, FRotator NewRot);

	UFUNCTION(NetMulticast, Reliable)
	void NewMulticast_SomeonePickedUpItem(bool bPickedUp);

	FVector _ItemHoldingOffset = FVector::ZeroVector;
	FVector _MoveToLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Spring")
		float _TorsionSpringStiffness = 50000.f;

	UPROPERTY(EditAnywhere, Category = "Spring")
		float _TorsionSpringDamping = 30.f;


	virtual void PhysicsTick_Implementation(float SubsetDeltaTime);

	FCalculateCustomPhysics _OnCalculateCustomPhysics;

	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

	UPROPERTY(EditAnywhere);
	UPhysicsHandlerComponent* _PHC;

};
