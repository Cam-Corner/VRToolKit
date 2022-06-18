// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Online/PhysicsReplicationComponent.h"
#include "Utility/PIDControllers.h"
#include "PhysicsHandlerComponent.generated.h"

class AController;

/* Who has control over the final say over this physics object
* @None means that all clients will their own say over the object and wont be sync through networking at all
* @Client authority will only work when the component is also owned by a client
*/
UENUM(BlueprintType)
enum EAuthorityType
{
	EAT_Server	UMETA("Server Authority"),
	EAT_Client	UMETA("Client Authority"),
	EAT_None	UMETA("No Authority")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UPhysicsHandlerComponent : /*public UActorComponent,*/ public UPhysicsReplicationComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPhysicsHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Set the authority type for this physics component only applys for networked games and when matching a target */
	UFUNCTION(BlueprintCallable)
	void SetMatchTargetAuthorityType(EAuthorityType MatchTargetAuthorityType);

	/* if match target is true then this is the object we will be trying to match */
	UFUNCTION(BlueprintCallable)
	void SetTargetObject(USceneComponent* Comp);

	/* Should we follow the target object */
	UFUNCTION(BlueprintCallable)
	void SetMatchTarget(bool bMatchTarget);

	/* Set the Player Controller that owns this object */
	UFUNCTION(BlueprintCallable)
	void SetPlayerControllerOwner(AController* PC);

	/* wether to enable or disable this component */
	UFUNCTION(BlueprintCallable)
	void EnableComponent(bool bEnabled);

	UFUNCTION(BlueprintCallable)
		void SetTargetLocationOffset(FVector Offset);

	UFUNCTION(BlueprintCallable)
		void SetTargetRotationOffset(FQuat Offset);

	void UpdateComponent(float DeltaTime);
protected:
	/* Who has default authority over this object when we are matching a target
	* @Only applies to match target functions
	* @When using the defualt physics system, the server has full control to sync up objects properly
	*/
	UPROPERTY(EditAnywhere, Category = "Replication Settings")
		TEnumAsByte<EAuthorityType> _MatchTargetAuthorityType;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
		FPDController3D _LocPD;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
		FQuatPDController _RotPD;
	
	/* should we match the specified target */
	bool _bMatchTarget = false;

private:
	void UpdateToNextPhysicsFrame();

	void PhysicsTick_DefaultPhysics(float SubsetDeltaTime);

	void PhysicsTick_MatchTarget(float SubsetDeltaTime);

	FCalculateCustomPhysics _OnCalculateCustomPhysics;

	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);
	
	USceneComponent* _TargetObject;

	AController* _PCOwner;

	bool _bComponentEnabled = true;

	FVector _Known_Goto_Location;
	FQuat _Known_Goto_Rotation;

	float _DefaultTickTimer = 1;
	float _TickTimer = 1;


	FVector _TargetLocationOffset = FVector::ZeroVector;

	FQuat _TargetRotationOffset = FQuat(FVector::ForwardVector, 0);
};
