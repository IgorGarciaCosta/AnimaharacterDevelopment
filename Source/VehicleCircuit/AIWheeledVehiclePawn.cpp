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

void AAIWheeledVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ManageVehicleSteering();
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

float AAIWheeledVehiclePawn::CalculateFowardVectorOffset(USplineComponent* Spline)
{
   //do some logic to make this work
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

void AAIWheeledVehiclePawn::ManageVehicleSteering()
{
    if (NextSplinePoint.IsZero()) return;


    FVector VehicleLocation = GetActorLocation();
    FVector TargetLocation = NextSplinePoint; // posição do próximo ponto

    FVector ForwardVector = GetActorForwardVector();
    FVector DirectionToTarget = (TargetLocation - VehicleLocation).GetSafeNormal();

    float Dot = FVector::DotProduct(ForwardVector, DirectionToTarget);
    FVector Cross = FVector::CrossProduct(ForwardVector, DirectionToTarget);

    // O valor do steering é baseado no sinal do componente Z do vetor cross product
    float SteeringValue = FMath::Clamp(Cross.Z, -1.0f, 1.0f);

    // Agora aplique o steering e throttle

    Steer(SteeringValue);
    

}
