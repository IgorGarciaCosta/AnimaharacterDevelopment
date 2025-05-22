#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Semaphore.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AAIWheeledVehiclePawn;

UCLASS()
class VEHICLECIRCUIT_API ASemaphore : public AActor
{
    GENERATED_BODY()

public:
    ASemaphore();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Componentes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SemaphoreMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* BoxCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    float SemaphoreBeginState = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "timers")
    float RedDelay = 10;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "timers")
    float YellowDelay = 5;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "timers")
    float GreenDelay = 5;


private:
    // Estado atual do semáforo (1=vermelho, 2=amarelo, 3=verde)
    int32 CurrentSemaphoreState;

    // Timer para controlar a troca de estado
    FTimerHandle SemaphoreTimerHandle;

    // Função que gerencia a máquina de estados
    void RunSemaphoreStateMachine();

    // Mapeia estado para tempo de delay
    float GetDelayForState(int32 State) const;

    // Guardar os veículos dentro do Box para comunicar status
    TSet<AAIWheeledVehiclePawn*> VehiclesInBox;

    // Envia o status atual do semaforo para todos veiculos dentro do box
    void SendSemaphoreStatusToVehicles(int32 Status);

public:
    // Evento Blueprint para mudar cor do semáforo
    UFUNCTION(BlueprintImplementableEvent, Category = "Semaphore")
    void ChangeSemaphoreColor(int32 SemaphoreState);

    // Evento Blueprint para ativar a travessia de pedestres
    UFUNCTION(BlueprintImplementableEvent, Category = "Semaphore")
    void MakePedestriansWalk();

private:
    // Funções para overlap
    UFUNCTION()
    void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
