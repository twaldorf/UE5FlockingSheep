#include "Characters/SheepCharacter.h"

#include <string>

#include "AIController.h"
#include "VectorTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MechJam23/MechJam23GameModeBase.h"
#include "Math/UnrealMathUtility.h"
#include "NiagaraFunctionLibrary.h"
#include "Environment/SheepPenZone.h"

// Sets default values
ASheepCharacter::ASheepCharacter() :
	BaseRunDistance(1000.f),
	MaxHealth(10),
	CurrentHealth(MaxHealth)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Tags.Add(FName("Sheep"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	SeparationThreshold = FMath::RandRange(10,200);
	Velocity = GetActorLocation();

}

void ASheepCharacter::HearSound(FVector& SoundLocation, float ThreatFactor)
{
	if (bInPen) return;

	PlayBaaSound();
	
	// Get the vector from the sound location to the sheep, normalize it to find direction sheep should run
	FVector Direction = GetActorLocation() - SoundLocation;
	Direction.Z = 0.f;
	Direction.Normalize();

	FVector RunToLocation = GetActorLocation() + (Direction * BaseRunDistance * ThreatFactor);

	Alarmed = true;
	Alarm = RunToLocation;
	Velocity = RunToLocation;
	Speed = MaxSpeed;
	GetCharacterMovement()->MaxWalkSpeed = Speed;

	AIController->MoveToLocation(RunToLocation, 5.f, false, true, true, false, nullptr, true);
	// DrawDebugSphere(GetWorld(), RunToLocation + FVector(0, 0, 100), 100, 5, FColor::Red, false, 2);
}

void ASheepCharacter::ArriveInPen()
{
	bInPen = true;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASheepPenZone::StaticClass(), Pens);
}

void ASheepCharacter::UpdateVector(const TArray<AActor*>& Flock, int FlockRadius)
{
	if (bInPen)
	{
		FVector PenVector = GetPenVector();
		Velocity = PenVector;
		AIController->MoveToLocation(PenVector, 5.f, false, true, true, false, nullptr, true);
				 DrawDebugSphere(GetWorld(), PenVector + FVector(0, 0, 100), 100, 5, FColor::Green, false, 2);
	} else
	{
		FVector AvgVelocity(0);
		float AvgSpeed = 0;
		FVector AvgPosition(0,0,0);

		// Away will face away from the nearest sheep
		FVector Away(0);

		// My position
		FVector Pos = GetActorLocation();

		// Allow alarm state to dominate behavior
		if (GetVelocity().Size() < 2)
		{
			Alarm = FVector(0);
			Alarmed = false;
			// Create a cohort of visible nearby sheep (shoop = not in my cohort, sheep = in my cohort)
			TArray<AActor*> Cohort = Flock.FilterByPredicate([this, &FlockRadius, Pos](AActor* Shoop)
			{
				if (Shoop != nullptr)
				{
					// Get the relative position of the shoop
					FVector ShoopPos = Shoop->GetActorLocation();
					FVector RelativePos = Shoop->GetActorLocation() - Pos;
					// Is the shoop in front of me?
					bool InView = FVector::DotProduct(GetActorForwardVector(), RelativePos) > -0.7;
					// Is the shoop within the flock radius?
					bool InRadius = GetDistanceTo(Shoop) < FlockRadius;
					// Am I the shoop?!
					bool Other = Shoop != this;
					return  InRadius && Other;
				}
				return false;
			});

			int CCount = Cohort.Num();

			// Check for flock members
			if (CCount > 0)
			{
				// Poll cohort for relevant data
				for (AActor* Shoop : Cohort)
				{
					ASheepCharacter* Sheep = Cast<ASheepCharacter>(Shoop);
					FVector SheepPos = Sheep->GetActorLocation();
					if (Sheep->Alarmed)
					{
						AvgVelocity += Sheep->Velocity;
						// TODO: AvgVelocity += Speed * GetWorldNormFromARelativeToB(Sheep->Velocity, GetActorLocation());		
					} else
					{
						AvgVelocity += Sheep->Velocity;
					}
					AvgSpeed += Sheep->Speed;
					AvgPosition += SheepPos / GetDistanceTo(Shoop);
					
					FVector FromShoop = Pos - SheepPos;
					// AwayWeight increases linearly from 0 to INF as distance from sheep decreases
					AwayWeight = SeparationThreshold / this->GetDistanceTo(Sheep);
					Away += FromShoop * static_cast<double>(AwayWeight);
				}

				// take averages
				AvgVelocity = AvgVelocity / CCount;
				AvgPosition = AvgPosition / CCount;

				FVector Displacement = AvgPosition - Pos;
				Displacement.Normalize();
				
				// Angle to cohort centroid
				float Theta = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), AvgPosition) / GetActorForwardVector().Size() * AvgPosition.Size());	
				
				float DifferenceFactor = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), Displacement)) / PI;

				FVector NewHeadingVector = AvgPosition + Away + (AvgVelocity + Velocity) / 2;
				// DrawDebugSphere(GetWorld(), NewHeadingVector + FVector(0, 0, 100), 100, 5, FColor::Green, false, 2);
				// DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0, 0, 100), 100, 5, FColor::Blue, false, 10);
				
				// Update Sheep movement
				Velocity = NewHeadingVector;
				Speed = FMath::Min(((NewHeadingVector - Pos).Size() + Speed ) / 2, MaxSpeed) * 0.99;
				GetCharacterMovement()->MaxWalkSpeed = Speed;
				AIController->MoveToLocation(NewHeadingVector, 5.f, false, true, true, false, nullptr, true);
			} else
			{
				FVector NewHeading = GetActorLocation() + FVector(FMath::RandRange(100,500), FMath::RandRange(100,500), 50);
				Speed = 100;
				GetCharacterMovement()->MaxWalkSpeed = Speed;
				AIController->MoveToLocation(NewHeading, 5.f, false, true, true, false, nullptr, true);
				// DrawDebugSphere(GetWorld(), NewHeading + FVector(0, 0, 100), 100, 5, FColor::Purple, false, 2);
			}
		}
	}
}

FVector ASheepCharacter::GetPenVector()
{
	FVector To(0,0,50);
	int X, Y = 0;
	for (auto const Pen : Pens)
	{
		FVector const Location = Pen->GetActorLocation();
		X = FMath::RandRange(Location.X - 200, Location.X + 200);
		Y = FMath::RandRange(Location.Y - 200, Location.Y + 200);
	}
	To.X = X;
	To.Y = Y;

	return To;
}

void ASheepCharacter::GetHit_Implementation(int32 Damage, FHitResult HitResult)
{
	if (BloodHitParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BloodHitParticles, HitResult.Location, HitResult.ImpactNormal.Rotation());
	}
	
	CurrentHealth -= Damage;
	if (CurrentHealth <= 0)
	{
		Die();
	}
}

// Called when the game starts or when spawned
void ASheepCharacter::BeginPlay()
{
	Super::BeginPlay();

	Heading = GetActorRotation();
	AIController = CastChecked<AAIController>(GetController());
	Velocity = GetActorLocation();

	if (AMechJam23GameModeBase* GameMode = CastChecked<AMechJam23GameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->IncrementTotalSheep();
	}
	
}

void ASheepCharacter::Die()
{
	if (AMechJam23GameModeBase* GameMode = CastChecked<AMechJam23GameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->IncrementSheepLost();
	}
	
	DeathFX();
	
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ASheepCharacter::DestroySheep, 2.f, false);
}

void ASheepCharacter::DestroySheep()
{
	Destroy();
}

void ASheepCharacter::PlayBaaSound()
{
	if (BaaSound)
	{
		int RandInt = FMath::RandRange(0, 2);
		if (RandInt == 0)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BaaSound, GetActorLocation());
		}
		
	}
}

