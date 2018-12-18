// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "PrototypeCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ALadder;

/**
 *  APrototypeCharacter extends APaperCharacter 
 *  @see ACharacter, APaperCharacter
 */
UCLASS()
class PROTOTYPE_API APrototypeCharacter : public APaperCharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	APrototypeCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* Camera;
	void MoveRight(float Value);
	void MoveUp(float Value);

	void Jump() override;

	void SetCharacterYaw(float Value);

	ALadder* Ladder;

	bool bIsClimbing;

	virtual bool CanJumpInternal_Implementation() const;

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void AnimationUpdate();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetLadder(ALadder*);

	ALadder* GetLadder() { return Ladder; }

	void SetIsClimbing(bool value);

	virtual void Landed(const FHitResult& Hit) override;	
};
