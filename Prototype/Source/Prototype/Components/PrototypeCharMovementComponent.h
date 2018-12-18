// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrototypeCharMovementComponent.generated.h"

enum ECustomMove {
	ClimbLadder = 1,
	ClimbRope = 2
};

//=============================================================================
/**
 * PrototypeCharMovementComponent handles movement logic for the associated PrototypeCharacter owner.
 * It supports all movement modes as standard CharacterMovementComponent and extends it with custom movement as 
 * climbing the ladder
 *
 * @see APrototypeCharacter, UCharacterMovementComponent
 */
UCLASS()
class PROTOTYPE_API UPrototypeCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
/**
 * Default UObject constructor.
 */
	UPrototypeCharMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The maximum character speed when he climbs ladder. */
	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxLadderClimbSpeed;

	/** The maximum character speed when he climbs rope. */
	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxRopeClimbSpeed;

	/** The maximum character speed when he climbs Rope. */
	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float LadderJumpZVelocity;

protected:
	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void PhysLadder(float deltaTime, int32 Iterations);

	virtual void PhysRope(float deltaTime, int32 Iterations);
	
	bool bMaintainVerticalClimbVelocity = true;
	
	void MaintainVerticalClimbVelocity();
	
	void MoveAlongLadder(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult);

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual bool IsClimbingLadder() const;

	virtual bool IsClimbingRope() const;

	virtual float GetMaxSpeed() const override;

	virtual bool DoJump(bool bReplayingMoves);

	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float LadderFriction;
};
