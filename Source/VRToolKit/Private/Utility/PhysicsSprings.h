#pragma once

#include "CoreMinimal.h"
#include "Utility/ExtraMaths.h"
#include "PhysicsSprings.generated.h"

USTRUCT(BlueprintType)
struct FLinearSpring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float SpringStiffnesss = 1.0f;

	UPROPERTY(EditAnywhere)
	float CriticalDamping = 0.5f;

	FVector Update(FVector CurrentLoc, FVector DesiredLoc, FVector CurrentVel, FVector DesiredVel)
	{
		FVector Diff = CurrentLoc - DesiredLoc;
		FVector VelDiff = CurrentVel - DesiredVel;

		FVector F = -SpringStiffnesss * Diff - CriticalDamping * VelDiff;

		return F;
	}
};

USTRUCT(BlueprintType)
struct FTorsionalSpring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float SpringStiffnesss = 1.0f;

	UPROPERTY(EditAnywhere)
	float CriticalDamping = 0.5f;

	FVector GetRequiredTorque(FVector CurrentRotation, FVector DesiredRotation, FVector CurrentVel, FVector DesiredVel)
	{
		float Distance = ExtraMaths::GetAngleOfTwoVectors(DesiredRotation, CurrentRotation );
		FVector AngleOfRot = FVector::CrossProduct(DesiredRotation, CurrentRotation);
		FVector AngleOfRotVel = FVector::CrossProduct(CurrentVel, DesiredVel);
		FVector VelDiff = DesiredVel - CurrentVel;

		FVector T = -SpringStiffnesss * Distance * AngleOfRot - CriticalDamping * VelDiff;
		return T;
	}
};

USTRUCT(BlueprintType)
struct FTorsionalSpringRot
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		float SpringStiffnesss = 1.0f;

	UPROPERTY(EditAnywhere)
		float ForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere)
		float CriticalDamping = 0.5f;

	FVector GetRequiredTorque(FQuat CurrentRotation, FQuat DesiredRotation, FVector CurrentVel, FVector DesiredVel)
	{
		FVector FinalT = FVector::ZeroVector;

		FinalT += CalculateAxisTorque(CurrentRotation.GetForwardVector(), DesiredRotation.GetForwardVector(),
			CurrentVel, DesiredVel);

		FinalT += CalculateAxisTorque(CurrentRotation.GetRightVector(), DesiredRotation.GetRightVector(),
			CurrentVel, DesiredVel);

		FinalT += CalculateAxisTorque(CurrentRotation.GetUpVector(), DesiredRotation.GetUpVector(),
			CurrentVel, DesiredVel);

		return FinalT;
	}

private:
	FVector CalculateAxisTorque(FVector CRot, FVector DRot, FVector CurrentVel, FVector DesiredVel)
	{
		float Distance = ExtraMaths::GetAngleOfTwoVectors(DRot, CRot);
		float Dist = 0 - Distance;
		FVector AngleOfRot = FVector::CrossProduct(DRot, CRot);
		FVector AngleOfRotVel = FVector::CrossProduct(CurrentVel, DesiredVel);
		FVector VelDiff = DesiredVel - CurrentVel;

		FVector T = -SpringStiffnesss * Dist * AngleOfRot - CriticalDamping * VelDiff * AngleOfRotVel;
		return T; //* ForceMultiplier;
	}
};

USTRUCT(BlueprintType)
struct FQuatSpring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float SpringStiffnesss = 1.0f;

	UPROPERTY(EditAnywhere)
		float CriticalDamping = 0.5f;

	FVector GetRequiredTorque(FVector CurrentRotation, FVector DesiredRotation, FVector CurrentVel, FVector DesiredVel)
	{
		float Distance = ExtraMaths::GetAngleOfTwoVectors(DesiredRotation, CurrentRotation);
		FVector AngleOfRot = FVector::CrossProduct(DesiredRotation, CurrentRotation);
		FVector AngleOfRotVel = FVector::CrossProduct(CurrentVel, DesiredVel);
		FVector VelDiff = CurrentVel - DesiredVel;

		FVector T = -SpringStiffnesss * Distance * AngleOfRot - CriticalDamping * VelDiff;
		return T;
	}
};

/*USTRUCT(BlueprintType)
struct FPIDControllerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		float ForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere)
		float Proportional = 0.05f;

	UPROPERTY(EditAnywhere)
		float Integral = 0.05f;

	UPROPERTY(EditAnywhere)
		float Derivative = 0.05f;

	FPIDControllerData operator/(const float& Value)
	{
		FPIDControllerData NewValues;
		NewValues.ForceMultiplier = ForceMultiplier / Value;
		NewValues.Derivative = Derivative / Value;
		NewValues.Integral = Integral / Value;
		NewValues.Proportional = Proportional / Value;
		return NewValues;
	}

};

USTRUCT(BlueprintType)
struct FPIDController
{
	GENERATED_BODY()

public:
	FPIDController() {}

	float Update(float DeltaTime, float CurrentValue, float DesiredValue)
	{
		float ThisError = DesiredValue - CurrentValue;
		float ThisIntegral = IntegralPrior + ThisError * DeltaTime;
		float ThisDerivative = (ThisError - ErrorPrior) / DeltaTime;
		float Output = (PIDData.Proportional * ErrorPrior) + (ThisIntegral * PIDData.Integral) + (ThisDerivative * PIDData.Derivative);

		ErrorPrior = ThisError;
		IntegralPrior = ThisIntegral;

		return Output * PIDData.ForceMultiplier;
	}
	*/
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	/*void SetPIDValue(FPIDControllerData InPIDData)
	{
		PIDData = InPIDData;
	}

	FPIDControllerData GetPIDSettings()
	{ 
		return  PIDData;
	}

protected:
	UPROPERTY(EditAnywhere)
		FPIDControllerData PIDData;

private:
	float ErrorPrior = 0;
	float IntegralPrior = 0;
};
*/
/*USTRUCT(BlueprintType)
struct FPIDController3D
{
	GENERATED_BODY()

public:
	FPIDController3D() {}
	*/
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	/*void SetPIDValue(FPIDControllerData InPIDData)
	{
		PIDData = InPIDData;
	}

	FVector Update(float DeltaTime, FVector CurrentValue, FVector DesiredValue)
	{
		FVector Result = FVector(0, 0, 0);

		XPID.SetPIDValue(PIDData);
		YPID.SetPIDValue(PIDData);
		ZPID.SetPIDValue(PIDData);

		Result.X = XPID.Update(DeltaTime, CurrentValue.X, DesiredValue.X);
		Result.Y = YPID.Update(DeltaTime, CurrentValue.Y, DesiredValue.Y);
		Result.Z = ZPID.Update(DeltaTime, CurrentValue.Z, DesiredValue.Z);

		return Result;
	}

	FPIDControllerData GetPIDSettings()
	{
		return  PIDData;
	}

protected:
	UPROPERTY(EditAnywhere)
		FPIDControllerData PIDData;

private:
	float ErrorPrior = 0;
	float IntegralPrior = 0;

	FPIDController XPID;
	FPIDController YPID;
	FPIDController ZPID;
};*/

/*USTRUCT(BlueprintType)
struct FQuatPDController
{
	GENERATED_BODY()

public:
	FQuatPDController() {}
	*/
	
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	/*void SetPIDValue(FPIDControllerData InPIDData)
	{
		PIDData = InPIDData;
	}*/

	/*
	* CQuat = Current Quaternion Value
	* DQuat = Desired Quaternion Value
	*/
	/*FVector Update(float DeltaTime, FQuat CQuat, FQuat DQuat, FVector AVel, FVector InertiaTensor)
	{
		FVector Axis = FVector::ZeroVector;
		float Angle = 0;
		FQuat Error = DQuat * CQuat.Inverse();

		if (Error.W < 0)
		{
			Error.X = -Error.X;
			Error.Y = -Error.Y;
			Error.Z = -Error.Z;
			Error.W = -Error.W;
		}

		Error.ToAxisAndAngle(Axis, Angle);
		Axis.Normalize();
		FVector::DegreesToRadians(Axis);
		FVector Value = PIDData.Proportional * Axis * Angle - PIDData.Derivative * AVel;
		FQuat RotInertia2World = InertiaTensor.ToOrientationQuat() * CQuat;
		Value = RotInertia2World.Inverse() * Value;
		Value *= InertiaTensor;
		Value = RotInertia2World * Value;
		return Value * PIDData.ForceMultiplier;
	}

	FPIDControllerData GetPIDSettings()
	{
		return  PIDData;
	}

protected:
	UPROPERTY(EditAnywhere)
		FPIDControllerData PIDData;
};
*/
