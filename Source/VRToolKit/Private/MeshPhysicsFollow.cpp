// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshPhysicsFollow.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMeshPhysicsFollow::AMeshPhysicsFollow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	_FollowMesh = CreateDefaultSubobject<UStaticMeshComponent>("FollowMesh");
	_FollowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = _FollowMesh;

	_PhysicsMesh = CreateDefaultSubobject<UStaticMeshComponent>("PhysicsMesh");
	_PhysicsMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_PhysicsMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	_PhysicsMesh->SetupAttachment(_FollowMesh);

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");
}

// Called when the game starts or when spawned
void AMeshPhysicsFollow::BeginPlay()
{
	Super::BeginPlay();

	_PHC->SetReplicatedPhysicsObject(_PhysicsMesh);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	_PHC->SetTargetObject(_FollowMesh);
	_PHC->SetMatchTarget(true);
	_PHC->SetPlayerControllerOwner(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

// Called every frame
void AMeshPhysicsFollow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

