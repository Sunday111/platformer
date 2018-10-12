// Fill out your copyright notice in the Description page of Project Settings.

#include "PrototypeCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Paper2D/Classes/PaperFlipbookComponent.h"
#include "GameFramework/Controller.h"

APrototypeCharacter::APrototypeCharacter()
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APrototypeCharacter::Jump);
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
}

void APrototypeCharacter::SetCharacterYaw(float Value)
{	
	GetController()->SetControlRotation(FRotator(0.0f, Value, 0.0f));
}

void APrototypeCharacter::Jump()
{
	Super::Jump();
}

