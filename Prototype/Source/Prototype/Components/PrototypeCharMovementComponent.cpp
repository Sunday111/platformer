// Fill out your copyright notice in the Description page of Project Settings.

#include "PrototypeCharMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
#include "Prototype/Actors/PrototypeCharacter.h"
#include "Prototype/Actors/Ladder.h"

//Char stats

DECLARE_CYCLE_STAT(TEXT("Char PhysLadder"), STAT_CharPhysLadder, STATGROUP_Character);

UPrototypeCharMovementComponent::UPrototypeCharMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxLadderClimbSpeed = 300.0f;
	MaxRopeClimbSpeed = 200.0f;
	SetWalkableFloorAngle(90.0f);
	LadderJumpZVelocity = 300.0f;
	LadderFriction = GroundFriction;
}

void UPrototypeCharMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	Super::SetMovementMode(NewMovementMode, NewCustomMode);

	if (NewMovementMode == EMovementMode::MOVE_Custom && NewCustomMode == ECustomMove::ClimbLadder)
	{
		StopActiveMovement();
		Velocity = FVector::ZeroVector;
		APrototypeCharacter* Proto = Cast<APrototypeCharacter>(GetOwner());
		if (Proto)
		{
			//TODO: Should 

			FVector v = Proto->GetActorLocation(); 
			v.X = Proto->GetLadder()->GetActorLocation().X;
			v.Y = Proto->GetLadder()->GetActorLocation().Y;

			Proto->SetActorLocation(v);
		}
	}
}

void UPrototypeCharMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	
}

void UPrototypeCharMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!CharacterOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
};

float UPrototypeCharMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case ECustomMove::ClimbLadder:
			return MaxLadderClimbSpeed;
			break;
		case ECustomMove::ClimbRope:
			return MaxRopeClimbSpeed;
			break;
		default:
			return 0.f;
			break;
		}
	} else
	{
		return Super::GetMaxSpeed();
	}
}

void UPrototypeCharMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (this->CustomMovementMode)
	{
	case ECustomMove::ClimbLadder:
		PhysLadder(deltaTime, Iterations);
		break;
	case ECustomMove::ClimbRope:
		PhysRope(deltaTime, Iterations);
		break;
	default:
		break;
	}
}

void UPrototypeCharMovementComponent::PhysLadder(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysLadder);


	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->Role != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Custom, ECustomMove::ClimbLadder);
		return;
	}

	//devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysLadder: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->Role == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		//const FFindLadderResult OldLadder = CurrentLadder;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainVerticalClimbVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration.X = 0.f;
		Acceleration.Y = 0.f;

		// Apply acceleration
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			CalcVelocity(timeTick, LadderFriction, false, GetMaxBrakingDeceleration());
			//devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}

		ApplyRootMotionToVelocity(timeTick);
		//devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if (IsFalling())
		{
			// Root motion could have put us into Falling.
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongLadder(MoveVelocity, timeTick, &StepDownResult);

			if (StepDownResult.FloorResult.IsWalkableFloor())
			{
				APrototypeCharacter* Proto = Cast<APrototypeCharacter>(GetOwner());
				if (Proto)
				{
					Proto->SetIsClimbing(false);
				}
			}

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsClimbingLadder())
	{
		MaintainVerticalClimbVelocity();
	}
}
void UPrototypeCharMovementComponent::PhysRope(float deltaTime, int32 Iterations)
{
	// as mock use flying physics
	PhysFlying(deltaTime, Iterations);
}

bool UPrototypeCharMovementComponent::IsClimbingLadder() const
{
	return (MovementMode == MOVE_Custom) && (CustomMovementMode == ECustomMove::ClimbLadder) && UpdatedComponent;
}

bool UPrototypeCharMovementComponent::IsClimbingRope() const
{
	return (MovementMode == MOVE_Custom) && (CustomMovementMode == ECustomMove::ClimbRope) && UpdatedComponent;
}

bool UPrototypeCharMovementComponent::DoJump(bool bReplayingMoves)
{
	if ((MovementMode == MOVE_Custom) && (CustomMovementMode == ECustomMove::ClimbLadder))
	{
		Velocity.Z = FMath::Max(Velocity.Z, JumpZVelocity);
		SetMovementMode(MOVE_Falling,0);
		return true;
	}

	return Super::DoJump(bReplayingMoves);
}

void UPrototypeCharMovementComponent::MaintainVerticalClimbVelocity()
{
	if (Velocity.X != 0.f || Velocity.Y != 0.f)
	{
		if (bMaintainVerticalClimbVelocity)
		{
			// Ramp movement already maintained the velocity, so we just want to remove the vertical component.
			Velocity.X = 0.f;
			Velocity.Y = 0.f;
		}
		else
		{
			// Rescale velocity to be horizontal but maintain magnitude of last update.
			Velocity.Z = Velocity.Size();
			Velocity.X = 0.f;
			Velocity.Y = 0.f;
		}
	}
}

void UPrototypeCharMovementComponent::MoveAlongLadder(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	const FVector Delta = FVector(0, 0, InVelocity.Z) * DeltaSeconds;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (InVelocity.Z < 0)
	{
		FindFloor(UpdatedComponent->GetComponentLocation(), OutStepDownResult->FloorResult, false, &Hit);
	}	
}
