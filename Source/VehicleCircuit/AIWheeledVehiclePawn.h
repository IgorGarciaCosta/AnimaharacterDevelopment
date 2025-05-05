#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Components/BoxComponent.h"
#include "AIWheeledVehiclePawn.generated.h"

UCLASS()
class VEHICLECIRCUIT_API AAIWheeledVehiclePawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    AAIWheeledVehiclePawn();


    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
    UBoxComponent* PreCollisionDetector;

    // Track incoming collision status
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    bool IncomingCollision = false;

    // Timer handle for retriggerable delay
    FTimerHandle IncomingCollisionTimerHandle;

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Retriggerable delay callback
    void ResetIncomingCollision();

    // Starts/reset the retriggerable delay timer
    void StartRetriggerableDelay(float Duration);
};
