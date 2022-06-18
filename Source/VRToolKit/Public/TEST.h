// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TEST.generated.h"

UCLASS()
class VRTOOLKIT_API ATEST : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATEST();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class USceneComponent* _Root;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* _FingerStartPoint;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* _ParmStartPoint;
};
