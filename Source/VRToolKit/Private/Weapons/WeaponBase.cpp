// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Items/VRGrabComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Player/VRHand.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//setup skeletal mesh
	_WeaponSKM = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh");
	RootComponent = _WeaponSKM;

	//setup primary grab component
	_WeaponPrimaryGrab = CreateDefaultSubobject<UVRGrabComponent>("Primary Grab Component");
	_WeaponPrimaryGrab->SetupAttachment(_WeaponSKM);
	_WeaponPrimaryGrab->_ComponentGrabbed.AddDynamic(this, &AWeaponBase::GrabbedWeapon);
	_WeaponPrimaryGrab->_ComponentReleased.AddDynamic(this, &AWeaponBase::ReleasedWeapon);

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	_PHC->SetReplicatedPhysicsObject(_WeaponSKM);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	//_PHC->SetTargetObject(_MotionControllerComp);
	_PHC->SetMatchTarget(false);
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (_WeaponPrimaryGrab->GetHand())
	{
		FTransform T = _WeaponPrimaryGrab->GetHand()->GetMotionControllerComponent()->GetComponentTransform();
		DrawDebugLine(GetWorld(), T.GetLocation(), T.GetLocation() + (T.GetRotation().GetForwardVector() * 5), FColor::Red, false, .1f);
		DrawDebugLine(GetWorld(), T.GetLocation(), T.GetLocation() + (T.GetRotation().GetRightVector() * 5), FColor::Green, false, .1f);
		DrawDebugLine(GetWorld(), T.GetLocation(), T.GetLocation() + (T.GetRotation().GetUpVector() * 5), FColor::Blue, false, .1f);

		FTransform T1 = _WeaponSKM->GetComponentTransform();
		DrawDebugLine(GetWorld(), T1.GetLocation(), T1.GetLocation() + (T1.GetRotation().GetForwardVector() * 5), FColor::Red, false, .1f);
		DrawDebugLine(GetWorld(), T1.GetLocation(), T1.GetLocation() + (T1.GetRotation().GetRightVector() * 5), FColor::Green, false, .1f);
		DrawDebugLine(GetWorld(), T1.GetLocation(), T1.GetLocation() + (T1.GetRotation().GetUpVector() * 5), FColor::Blue, false, .1f);
	}*/
	
	EditWeaponOffset(DeltaTime);
}

void AWeaponBase::EditWeaponOffset(float DeltaTime)
{
	if (_WeaponPrimaryGrab && _WeaponPrimaryGrab->GetHand())
	{
		FQuat HandRot = _WeaponPrimaryGrab->GetHand()->GetTrackingHandTransform().GetRotation();
		FQuat Offset = FQuat(FVector::CrossProduct(HandRot.GetForwardVector(), HandRot.GetUpVector()), -45);
		_PHC->SetTargetRotationOffset(Offset);
	}
}

void AWeaponBase::GrabbedWeapon(AVRHand* Hand)
{
	SetOwner(Hand->GetOwner());

	_PHC->SetTargetObject(Hand->GetMotionControllerComponent());
	_PHC->SetPlayerControllerOwner(Cast<APlayerController>(GetOwner()));
	_PHC->EnableComponent(true);
	_PHC->SetMatchTarget(true);

	/*if (GetLocalRole() >= ENetRole::ROLE_Authority)
	{
		NewMulticast_SomeonePickedUpItem(true);
	}*/
}

void AWeaponBase::ReleasedWeapon()
{
	_PHC->EnableComponent(false);
	_PHC->SetTargetObject(NULL);
	_PHC->SetPlayerControllerOwner(NULL);
	_PHC->SetMatchTarget(false);
	_PHC->SetTargetRotationOffset(FQuat(FVector::CrossProduct(FVector::ForwardVector, FVector::RightVector), 0));
	/*if (GetLocalRole() >= ENetRole::ROLE_Authority)
	{
		NewMulticast_SomeonePickedUpItem(false);
	}*/

	SetOwner(NULL);
}

