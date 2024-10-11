#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "SheepCharacter.generated.h"

class AAIController;
class UNiagaraSystem;

UCLASS()
class MECHJAM23_API ASheepCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASheepCharacter();

	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	void HearSound(FVector& SoundLocation, float ThreatFactor);

	void ArriveInPen();

	void UpdateVector(const TArray<AActor*> &Flock, int FlockRadius);
	FVector GetPenVector();

	// Interface Functions //
	void GetHit_Implementation(int32 Damage, FHitResult HitResult) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Die();

	UFUNCTION(BlueprintImplementableEvent)
	void DeathFX();
	
	void DestroySheep();

	void PlayBaaSound();

private:

	// set to True when the sheep is in the pen, to hide them from Wolves and TODO: passive wandering in pen behavior
	UPROPERTY(VisibleInstanceOnly, meta = (AllowPrivateAccess = "true"))
	bool bInPen = false;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> Pens;
	
	UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float BaseRunDistance;
	
	UPROPERTY(EditAnywhere, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	int32 MaxHealth;

	UPROPERTY(VisibleInstanceOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	int32 CurrentHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxSpeed = 800;

	UPROPERTY(VisibleInstanceOnly, meta = (AllowPrivateAccess = "true"))
	FRotator Heading;

	UPROPERTY(VisibleInstanceOnly, meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector(0);

	UPROPERTY(EditAnywhere)
	float CentroidWeight = 0.01;

	UPROPERTY(EditAnywhere)
	float AwayWeight = 0.1;

	UPROPERTY(EditAnywhere)
	FVector Alarm = FVector(0);

	UPROPERTY(EditAnywhere)
	bool Alarmed = false;

	UPROPERTY(EditAnywhere)
	float Speed = 0;

	UPROPERTY(VisibleAnywhere)
	int SeparationThreshold = 200; 

	UPROPERTY(EditAnywhere)
	TObjectPtr<AAIController> AIController;
	
	FTimerHandle DestroyTimer;

	// Sound //
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* BaaSound;

	// Particles //
	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UNiagaraSystem> BloodHitParticles;

public:

	FORCEINLINE USoundBase* GetDeathSound() const { return DeathSound; }
};
