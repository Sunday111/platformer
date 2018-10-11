// Fill out your copyright notice in the Description page of Project Settings.
#include "Lift.h"
#include "../../Engine/Plugins/2D/Paper2D/Source/Paper2D/Classes/PaperSpriteComponent.h"
#include "TimerManager.h"

// Sets default values
ALift::ALift()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
    Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));

    RootComponent = Scene;
}

// Called when the game starts or when spawned
void ALift::BeginPlay()
{
    Super::BeginPlay();

    if (MoveCurve)
    {
        Timeline = NewObject<UTimelineComponent>(this, FName("Timeline"));

        // Indicate it comes from a blueprint so it gets cleared when we rerun construction scripts
        Timeline->CreationMethod = EComponentCreationMethod::Native;

        // Add to array so it gets saved
        BlueprintCreatedComponents.Add(Timeline);

        // This component has a stable name that can be referenced for replication
        Timeline->SetNetAddressable();

        // Set which object the timeline should drive properties on
        Timeline->SetPropertySetObject(this);

        // Set which object the timeline should drive properties on
        Timeline->SetDirectionPropertyName(FName("TimelineDirection"));
        Timeline->SetPropertySetObject(this);

        Timeline->SetLooping(false);
        Timeline->SetTimelineLength(1.0f);
        Timeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
        Timeline->SetPlaybackPosition(0.0f, false);

        //Add the float curve to the timeline and connect it to your timelines's interpolation function
        {

            FOnTimelineFloat onTimelineCallback;
            onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineCallback") });
            Timeline->AddInterpFloat(MoveCurve, onTimelineCallback);
        }

        {
            FOnTimelineEventStatic onTimelineFinishedCallback;
            onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });
            Timeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);
        }

        Timeline->RegisterComponent();

        {// Set play rate
            float length;
            {
                FVector direction;
                RelativeEndLocation.ToDirectionAndLength(direction, length);
            }

            Timeline->SetPlayRate(1 / TravelTime);
        }

        Play();
    }
}

// Called every frame
void ALift::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (Timeline != NULL)
    {
        Timeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
    }
}

void ALift::Play()
{
    if (TimelineDirection == ETimelineDirection::Forward)
    {
        Timeline->Reverse();
    }
    else
    {
        Timeline->Play();
    }
}

void ALift::TimelineCallback(float val)
{
    Sprite->SetRelativeLocation(val * RelativeEndLocation);
}

void ALift::TimelineFinishedCallback()
{
    GetWorldTimerManager().SetTimer(m_stopTimer, this, &ALift::Play, StopTime);
}

