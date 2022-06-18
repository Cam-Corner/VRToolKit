// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class UVRGrabComponent;
class UPhysicsHandlerComponent;

UCLASS()
class VRTOOLKIT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Skeletal Mesh")
	USkeletalMeshComponent* _WeaponSKM;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Primary Grab")
	UVRGrabComponent* _WeaponPrimaryGrab;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Physics")
	UPhysicsHandlerComponent* _PHC;

	virtual void EditWeaponOffset(float DeltaTime);
protected:
	UFUNCTION()
	virtual void GrabbedWeapon(AVRHand* Hand);

	UFUNCTION()
	virtual void ReleasedWeapon();
};
