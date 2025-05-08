#include "AIWheeledVehiclePawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

AAIWheeledVehiclePawn::AAIWheeledVehiclePawn()
{
    PreCollisionDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("PreCollisionDetector"));
    PreCollisionDetector->SetupAttachment(Mesh);

    PreCollisionDetector->SetBoxExtent(FVector(200.f, 200.f, 100.f));
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

    PressBrake(1);

    // Start retriggerable delay with random duration between 3 and 7 seconds
    float RandomDuration = FMath::FRandRange(3.0f, 7.0f);
    StartRetriggerableDelay(RandomDuration);
}

void AAIWheeledVehiclePawn::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Collision ended with %s"), *GetNameSafe(OtherActor));
    IncomingCollision = false;  

    ReleaseBrake(0);
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


void AAIWheeledVehiclePawn::ControlSpeed(USplineComponent* Spline, float DeltaTime)
{
    if (!Spline || NextSplinePoint.IsZero() || IncomingCollision)
    {
        Accelerate(0.f);
        return;
    }

    // Obtem o quadro "até 300 unidades na frente"
    float ForwardOffset = 800.f;

    float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(GetActorLocation());
    float DistanceAlongSpline = Spline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);
    float SplineLength = Spline->GetSplineLength();

    float TargetDistance = DistanceAlongSpline + ForwardOffset;
    if (TargetDistance > SplineLength) TargetDistance -= SplineLength;

    // Obter localização exata no spline
    FVector PointAhead = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    DrawDebugSphere(
        GetWorld(),
        PointAhead,
        50.f,            // Raio da esfera: 50 unidades
        12,              // Número de segmentos da esfera
        FColor::Red,     // Cor vermelha para chamar atenção
        false,           // Não persistente, aparece só uma frame
        0.f,             // Duração de 5 segundos para visualização
        0,               // Profundidade padrão
        1.5f             // Espessura da linha
    );

    FRotator RotationAtPoint = Spline->GetRotationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    float CurrentYaw = RotationAtPoint.Yaw;

    // Calcula a diferença absoluta entre o yaw atual e o último
    float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(LastTargetYaw, CurrentYaw));
    // Normaliza YawDifference [0, MaxYawDiff] para [0, 1]
    float NormalizedYawDiff = FMath::Clamp(YawDifference, 0.f, .4f);
    
    // Inverte o valor: ângulo pequeno -> throttle alto, ângulo grande -> throttle baixo
    float TargetThrottle = FMath::Abs((.4 - NormalizedYawDiff));
    //if(TargetThrottle>=0.5)TargetThrottle -=0.4;

    // Atualiza para próximo tick
    LastTargetYaw = CurrentYaw;

    

    // Diferenciar taxa de variação para desacelerar e acelerar
    float changeRate;

    if (TargetThrottle < PriorThrottle) // Está desacelerando
    {
        changeRate = 3.f; // taxa maior de variação, desacelera rápido
    }
    else // Está acelerando
    {
        changeRate = 1.f;  // taxa normal
    }

    float FinalThrottle = PriorThrottle - FMath::Abs((PriorThrottle-TargetThrottle)*changeRate);
    if (FinalThrottle <= 0)FinalThrottle = 0.001;
    Accelerate(FinalThrottle);

    PriorThrottle = TargetThrottle;
    UE_LOG(LogTemp, Log, TEXT("AdaptiveSpeed: YawDiff=%.2f,NormalizedYawDiff=%.2f, Throttle=%.3f"), YawDifference, NormalizedYawDiff, FinalThrottle);
}

void AAIWheeledVehiclePawn::ResetIncomingCollision()
{
    
    if (IncomingCollision) {
        // Start retriggerable delay with random duration between 3 and 7 seconds
        float RandomDuration = FMath::FRandRange(3.0f, 7.0f);
        StartRetriggerableDelay(RandomDuration);
    }
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
