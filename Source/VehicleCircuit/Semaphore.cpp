// Fill out your copyright notice in the Description page of Project Settings.


#include "Semaphore.h"

// Sets default values
ASemaphore::ASemaphore()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASemaphore::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASemaphore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

