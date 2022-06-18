// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRGrabComponent.h"
#include "Online/NetworkHelpers.h"

// Sets default values for this component's properties
UVRGrabComponent::UVRGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	BoxExtent = FVector(1, 1, 1);


	// ...
}


// Called when the game starts
void UVRGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UVRGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UVRGrabComponent::GrabComponent(AVRHand* HandGrabbing)
{
	if (!_bGrabEnabled || _HandHoldingComponent)
		return false;

	_HandHoldingComponent = HandGrabbing;

	_ComponentGrabbed.Broadcast(HandGrabbing);

	return true;
}

void UVRGrabComponent::ReleaseComponent()
{
	_HandHoldingComponent = NULL;

	_ComponentReleased.Broadcast();
}

bool UVRGrabComponent::CanGrabComponent()
{
	if (_bGrabEnabled && !_HandHoldingComponent)
		return true;

	return false;
}

void UVRGrabComponent::TriggerPressed(float Value)
{
	_ComponentTrigger.Broadcast(Value);
}

void UVRGrabComponent::TopButtonPressed(bool bPressed)
{
	_ComponentTopButton.Broadcast(bPressed);
}

void UVRGrabComponent::BottomButtonPressed(bool bPressed)
{
	_ComponentBottomButton.Broadcast(bPressed);
}

void UVRGrabComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRGrabComponent, _bGrabEnabled);
	DOREPLIFETIME(UVRGrabComponent, _HandHoldingComponent);
}
