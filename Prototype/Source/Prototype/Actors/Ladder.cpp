// Fill out your copyright notice in the Description page of Project Settings.

#include "Ladder.h"
#include "Paper2D/Classes/PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrototypeCharacter.h"

// Sets default values
ALadder::ALadder()
{
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(Sprite);
	RootComponent = Sprite;

	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	Box->SetBoxExtent(FVector(100.0f));
	Box->OnComponentBeginOverlap.AddDynamic(this, &ALadder::HandleBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &ALadder::HandleEndOverlap);
}

// Called when the game starts or when spawned
void ALadder::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALadder::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("Overlapped BEGIN with Ladder"));

	if (OtherActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Actor found"));
		if (APrototypeCharacter* c = Cast<APrototypeCharacter>(OtherActor))
		{
			UE_LOG(LogTemp, Log, TEXT("Cast to MyProjectCharacter is successful"));
			c->SetLadder(this);
		}
	}
}

void ALadder::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APrototypeCharacter* c = Cast<APrototypeCharacter>(OtherActor))
	{
		c->SetLadder(nullptr);
	}
}

