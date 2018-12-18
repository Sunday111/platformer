// Fill out your copyright notice in the Description page of Project Settings.

#include "PrototypeCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Paper2D/Classes/PaperFlipbookComponent.h"
#include "GameFramework/Controller.h"
#include "Prototype/Components/PrototypeCharMovementComponent.h"

APrototypeCharacter::APrototypeCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPrototypeCharMovementComponent>(ACharacter::CharacterMovementComponentName)),
	Ladder(nullptr),
	bIsClimbing(false)
{
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraBoom);
}

void APrototypeCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetSprite()->SetIsReplicated(true);

}

// Called to bind functionality to input
void APrototypeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Move Right", this, &APrototypeCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Move Up", this, &APrototypeCharacter::MoveUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APrototypeCharacter::Jump);
}

void APrototypeCharacter::SetIsClimbing(bool value)
{
	bIsClimbing = value;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void APrototypeCharacter::SetLadder(ALadder * Ladder)
{
	this->Ladder = Ladder;
	if (!Ladder)
	{
		if (bIsClimbing)
		{
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
		bIsClimbing = false;
		
	}
}

void APrototypeCharacter::Landed(const FHitResult & Hit)
{
	Super::Landed(Hit);
}

void APrototypeCharacter::MoveRight(float Value)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);

	if (Value != 0.0f)
	{
		if (Value > 0.0f)
		{
			SetCharacterYaw(0.0f);
		}
		else
		{
			SetCharacterYaw(-180.0f);
		}
	}
	AnimationUpdate();
}

void APrototypeCharacter::MoveUp(float Value)
{
	if (!Value)
	{
		return;
	}

	if (bIsClimbing)
	{
		AddMovementInput(FVector(0.0f, 0.0f, 1.0f), Value);
		return;
	}
	else if (Ladder)
	{
		bIsClimbing = true;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Custom, ECustomMove::ClimbLadder);
		AddMovementInput(FVector(0.0f, 0.0f, 1.0f), Value);
	}
	else if (Value>0.0f)
	{
		Jump();
		return;
	}
	AnimationUpdate();
}

void APrototypeCharacter::SetCharacterYaw(float Value)
{	
	GetController()->SetControlRotation(FRotator(0.0f, Value, 0.0f));
}

bool APrototypeCharacter::CanJumpInternal_Implementation() const
{
	// Ensure the character isn't currently crouched.
	bool bCanJump = !bIsCrouched;

	if (UPrototypeCharMovementComponent* CharMoveComp = Cast<UPrototypeCharMovementComponent>(GetCharacterMovement()))
	{// Ensure that the CharacterMovement state is valid
		bCanJump &= CharMoveComp->IsJumpAllowed() &&
			!CharMoveComp->bWantsToCrouch &&
			// Can only jump from the ground, or multi-jump if already falling.
			(CharMoveComp->IsMovingOnGround() || CharMoveComp->IsFalling() || CharMoveComp->IsClimbingLadder());
	
		if (bCanJump)
		{
			// Ensure JumpHoldTime and JumpCount are valid.
			if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
			{
				if (JumpCurrentCount == 0 && CharMoveComp->IsFalling())
				{
					bCanJump = JumpCurrentCount + 1 < JumpMaxCount;
				}
				else
				{
					bCanJump = JumpCurrentCount < JumpMaxCount;
				}
			}
			else
			{
				// Only consider JumpKeyHoldTime as long as:
				// A) The jump limit hasn't been met OR
				// B) The jump limit has been met AND we were already jumping
				const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
				bCanJump = bJumpKeyHeld &&
					((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
			}
		}
	}

	return bCanJump;
}

void APrototypeCharacter::Jump()
{
	bIsClimbing = false;
	Super::Jump();
	AnimationUpdate();
}

