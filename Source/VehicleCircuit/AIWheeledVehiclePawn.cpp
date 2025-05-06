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
    return 1200;
}

void AAIWheeledVehiclePawn::FindNextTargetPoint(USplineComponent* Spline)
{
    if (!Spline) return;

    // Calcula offset frontal dinâmico (você tem essa função)
    float ForwardVectorOffset = 300;// CalculateFowardVectorOffset(Spline);

    // Ponto ajustado para frente da posição do ator
    FVector VehicleWithForwardOffset = GetActorLocation() + (GetActorForwardVector() * ForwardVectorOffset);

    // Encontra a distância mais próxima da posição com offset à frente
    float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(VehicleWithForwardOffset);
    float DistanceAlongSpline = Spline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);


    // Define distância do próximo ponto ~  ForwardVectorOffset (pode ajustar esse valor para controle de quão à frente olhar)
    float NextDistance;

    // Obter o comprimento total da spline para controle de wrap
    float SplineLength = Spline->GetSplineLength();

    if (bMoveClockwise)
    {
        //bDriveOnRightLane = !bDriveOnRightLane;
        // Avança na spline no sentido horário
        NextDistance = DistanceAlongSpline + ForwardVectorOffset;
        if (NextDistance > SplineLength)
        {
            NextDistance -= SplineLength; // wrap-around
        }
    }
    else
    {
        // Move para trás na spline (anti-horário)
        NextDistance = DistanceAlongSpline - ForwardVectorOffset;
        if (NextDistance < 0.f)
        {
            NextDistance += SplineLength; // wrap-around
        }
    }

    // Obtém a posição do próximo ponto na spline baseado na distância calculada
    FVector NextLoc = Spline->GetLocationAtDistanceAlongSpline(NextDistance, ESplineCoordinateSpace::World);

    // Multiplicador fixo para distância lateral (igual antes)
    float LateralOffsetAmount = 0.f;
    bDriveOnRightLane ? LateralOffsetAmount = 400.f : LateralOffsetAmount = -400.f;

    // Ajusta lateralidade — direita ou esquerda
    FVector LateralOffset = GetActorRightVector() * LateralOffsetAmount;

    // Se estiver no sentido antihorário, inverta o lado da lateralização para fazer sentido na pista
    if (!bMoveClockwise)
    {
        LateralOffset *= -1.f;
    }

    // Aplicar offset lateral no ponto
    NextLoc += LateralOffset;

    NextSplinePoint = NextLoc;

    // Para debug visual
    DrawDebugSphere(
        GetWorld(),
        NextLoc,
        100,
        12,
        FColor::Blue,
        false,
        0,
        0,
        1.0f
    );

    /*FVector VehicleWithForwardOffset = GetActorLocation() + (GetActorForwardVector() * CalculateFowardVectorOffset(Spline));

    FVector NextLoc = Spline->FindLocationClosestToWorldLocation(VehicleWithForwardOffset, ESplineCoordinateSpace::World);

    float MultiplyFactor = 400;
    if (bDriveOnRightLane) {
        NextLoc = NextLoc+(GetActorRightVector() * MultiplyFactor);
    }
    else {
        NextLoc = NextLoc + (GetActorRightVector() * (MultiplyFactor*-1));
    }

    NextSplinePoint = NextLoc;

    DrawDebugSphere(
        GetWorld(),
        NextLoc,
        100,
        12,
        FColor::Blue,
        false,
        0,
        0,
        1.0f
    );*/

    
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
