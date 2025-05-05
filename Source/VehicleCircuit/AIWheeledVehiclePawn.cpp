#include "AIWheeledVehiclePawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

AAIWheeledVehiclePawn::AAIWheeledVehiclePawn()
{
    PreCollisionDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("PreCollisionDetector"));
    PreCollisionDetector->SetupAttachment(Mesh);

    PreCollisionDetector->SetBoxExtent(FVector(200.f, 200.f, 100.f));
    PreCollisionDetector->SetCollisionProfileName(TEXT("Trigger"));
    PreCollisionDetector->SetGenerateOverlapEvents(true);


    IncomingCollision = false;
}

void AAIWheeledVehiclePawn::BeginPlay()
{
    Super::BeginPlay();
    PreCollisionDetector->OnComponentBeginOverlap.AddDynamic(this, &AAIWheeledVehiclePawn::OnBeginOverlap);
    PreCollisionDetector->OnComponentEndOverlap.AddDynamic(this, &AAIWheeledVehiclePawn::OnEndOverlap);
}

void AAIWheeledVehiclePawn::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Set collision flag true
    IncomingCollision = true;
    UE_LOG(LogTemp, Log, TEXT("Collision began with %s, IncomingCollision set to TRUE"), *GetNameSafe(OtherActor));

    // Start retriggerable delay with random duration between 3 and 7 seconds
    float RandomDuration = FMath::FRandRange(3.0f, 7.0f);
    StartRetriggerableDelay(RandomDuration);
}

void AAIWheeledVehiclePawn::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Collision ended with %s"), *GetNameSafe(OtherActor));
    // Optionally handle end overlap if needed
}

void AAIWheeledVehiclePawn::ResetIncomingCollision()
{
    IncomingCollision = false;
    UE_LOG(LogTemp, Log, TEXT("Retriggerable delay ended, IncomingCollision set to FALSE"));
}

void AAIWheeledVehiclePawn::StartRetriggerableDelay(float Duration)
{
    if (GetWorld())
    {
        // If timer is already active, clear it to retrigger delay
        if (GetWorld()->GetTimerManager().IsTimerActive(IncomingCollisionTimerHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(IncomingCollisionTimerHandle);
        }

        // Start new timer
        GetWorld()->GetTimerManager().SetTimer(IncomingCollisionTimerHandle, this, &AAIWheeledVehiclePawn::ResetIncomingCollision, Duration, false);
    }
}
