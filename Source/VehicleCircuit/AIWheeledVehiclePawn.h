#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIWheeledVehiclePawn.generated.h"

UCLASS()
class VEHICLECIRCUIT_API AAIWheeledVehiclePawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    AAIWheeledVehiclePawn();


    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
    UBoxComponent* PreCollisionDetector;

    // Track incoming collision status
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    bool IncomingCollision = false;

    bool bIsWaitingSemaphore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
    FVector NextSplinePoint = FVector(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
    bool bDriveOnRightLane = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
    bool bMoveClockwise = true;

    // Timer handle for retriggerable delay
    FTimerHandle IncomingCollisionTimerHandle;

    UFUNCTION(BlueprintImplementableEvent)
    void Steer(float steer);

    UFUNCTION(BlueprintImplementableEvent)
    void Accelerate(float speed);

    UFUNCTION(BlueprintImplementableEvent)
    void PressBrake(float value);

    UFUNCTION(BlueprintImplementableEvent)
    void PressHandBrake(bool IsPressed);

    UFUNCTION(BlueprintImplementableEvent)
    void ReleaseBrake();

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable)
    float CalculateFowardVectorOffset(USplineComponent* Spline);

    UFUNCTION(BlueprintCallable)
    void FindNextTargetPoint(USplineComponent* Spline);

    UFUNCTION(BlueprintCallable)
    void ControlSpeed(USplineComponent* Spline, float DeltaTime);

    // Retriggerable delay callback
    void ResetIncomingCollision();

    // Starts/reset the retriggerable delay timer
    void StartRetriggerableDelay(float Duration);

    void SetSemaphoreValue(int32 Status);
private:
    float LastTargetYaw = 0.f;
    float PriorThrottle = 0.f;
    float OldThrottle = 0;
    void ManageVehicleSteering();

    
};
