#include "Semaphore.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "AIWheeledVehiclePawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Sets default values
ASemaphore::ASemaphore()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar mesh estático do semáforo
    SemaphoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SemaphoreMesh"));
    RootComponent = SemaphoreMesh;

    // Criar BoxCollision para detectar veículos
    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    BoxCollision->SetupAttachment(RootComponent);
    BoxCollision->SetBoxExtent(FVector(300.f, 300.f, 300.f)); // Ajuste conforme necessário
    BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
    BoxCollision->SetGenerateOverlapEvents(true);

    
}

void ASemaphore::BeginPlay()
{
    Super::BeginPlay();

    // Começa com estado aleatório de 1 a 3
    CurrentSemaphoreState = FMath::RandRange(1, 3);
    ChangeSemaphoreColor(CurrentSemaphoreState);
    SendSemaphoreStatusToVehicles(CurrentSemaphoreState);

    // Inicia a máquina de estados
    RunSemaphoreStateMachine();

    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ASemaphore::OnBoxBeginOverlap);
    BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ASemaphore::OnBoxEndOverlap);
}

void ASemaphore::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASemaphore::RunSemaphoreStateMachine()
{
    // Define a transição de estados: 1 → 3 → 2 → 1 ...
    switch (CurrentSemaphoreState)
    {
    case 1: CurrentSemaphoreState = 3; break; // vermelho → verde
    case 3: CurrentSemaphoreState = 2; break; // verde → amarelo
    case 2: CurrentSemaphoreState = 1; break; // amarelo → vermelho
    default: CurrentSemaphoreState = 1; break; // fallback
    }

    ChangeSemaphoreColor(CurrentSemaphoreState);
    SendSemaphoreStatusToVehicles(CurrentSemaphoreState);

    if (CurrentSemaphoreState == 3) // verde → pedestres podem andar
    {
        MakePedestriansWalk();
    }

    float Delay = GetDelayForState(CurrentSemaphoreState);
    GetWorldTimerManager().SetTimer(SemaphoreTimerHandle, this, &ASemaphore::RunSemaphoreStateMachine, Delay, false);
}


float ASemaphore::GetDelayForState(int32 State) const
{
    switch (State)
    {
    case 1: // Vermelho
        return RedDelay;
    case 2: // Amarelo
        return YellowDelay;
    case 3: // Verde
        return GreenDelay;
    default:
        return 3.f;
    }
}

void ASemaphore::SendSemaphoreStatusToVehicles(int32 Status)
{
    for (AAIWheeledVehiclePawn* Vehicle : VehiclesInBox)
    {
        if (Vehicle)
        {
            Vehicle->SetSemaphoreValue(Status);
        }
    }
}

// Quando veículo entra na área
void ASemaphore::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    AAIWheeledVehiclePawn* Vehicle = Cast<AAIWheeledVehiclePawn>(OtherActor);
    if (Vehicle)
    {
        VehiclesInBox.Add(Vehicle);
        SendSemaphoreStatusToVehicles(CurrentSemaphoreState); // envia status atualizado para todos
    }
}

// Quando veículo sai da área
void ASemaphore::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AAIWheeledVehiclePawn* Vehicle = Cast<AAIWheeledVehiclePawn>(OtherActor);
    if (Vehicle)
    {
        Vehicle->SetSemaphoreValue(0); // resetar status no veículo
        VehiclesInBox.Remove(Vehicle);
    }
}
