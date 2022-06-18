// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "VRGrabComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGrabGrabbed, class AVRHand*, Hand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVRGrabReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGrabButtonPress, bool, bPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGrabAxis, float, Value);

class AVRHand;

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTOOLKIT_API UVRGrabComponent : public UBoxComponent
{
	GENERATED_BODY()
public:
	UVRGrabComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetGrabEnabled(bool bGrabbable) { _bGrabEnabled = bGrabbable; }

	bool GrabComponent(AVRHand* HandGrabbing);

	void ReleaseComponent();

	bool CanGrabComponent();

	FVRGrabGrabbed _ComponentGrabbed;

	FVRGrabReleased _ComponentReleased;

	FVRGrabButtonPress _ComponentTopButton;

	FVRGrabButtonPress _ComponentBottomButton;

	FVRGrabAxis _ComponentTrigger;

	void TriggerPressed(float Value);

	void TopButtonPressed(bool bPressed);

	void BottomButtonPressed(bool bPressed);

	AVRHand* GetHand() { return _HandHoldingComponent; }
private:
	UPROPERTY(Replicated)
		bool _bGrabEnabled = true;

	UPROPERTY(Replicated)
		AVRHand* _HandHoldingComponent;
};
