// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Lift.generated.h"

class UPaperSpriteComponent;
class UTimelineComponent;

UCLASS()
class PROTOTYPE_API ALift : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ALift();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void Play();

    UFUNCTION()
    void TimelineCallback(float val);

    UFUNCTION()
    void TimelineFinishedCallback();

    UPROPERTY()
    TEnumAsByte<ETimelineDirection::Type> TimelineDirection;

protected:
    UPROPERTY()
    USceneComponent* Scene = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UPaperSpriteComponent* Sprite = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UTimelineComponent* Timeline = nullptr;

    UPROPERTY(EditInstanceOnly, Category = "Parameters")
    float TravelTime = 1.0f;

    UPROPERTY(EditInstanceOnly, Category = "Parameters")
    float StopTime = 1.0f;

    UPROPERTY(EditInstanceOnly, Category = "Parameters", Meta = (MakeEditWidget = true))
    FVector RelativeEndLocation = FVector{ 1.0f, 1.0f, 1.0f};

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curve)
    UCurveFloat* MoveCurve = nullptr;

private:
    FTimerHandle m_stopTimer;
};
