// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplodeGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"


// Sets default values for this component's properties
UExplodeGrenade::UExplodeGrenade()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Path"));
	SplinePath->SetUnselectedSplineSegmentColor(FLinearColor::Red);

	// ...
}

// Called when the game starts
void UExplodeGrenade::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();

	//show debug spline
	GEngine->Exec( GetWorld(), TEXT( "show splines" ) );
	SplinePath->SetHiddenInGame(true);
	
}

// Called every frame
void UExplodeGrenade::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Storing player location and rotation for other usage
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(PlayerViewpointLocation, PlayerViewpointRotation);

	//Set debug line follow the player
	SplinePath->SetWorldLocationAndRotation(PlayerViewpointLocation, PlayerViewpointRotation);

	//Set line trace
	FVector LineTraceEnd = PlayerViewpointLocation + (PlayerViewpointRotation.Vector()*SearchScale);
	//If the physics handle is attached
	//Move the object we are holding every tick
	if(PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->SetTargetLocation(LineTraceEnd);
	}
	
	// ...
}

void UExplodeGrenade::FindPhysicsHandleComponent()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if(PhysicsHandle)
	{
		//Physics Handle Found
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing Physics Handle Component"));
	}
}

void UExplodeGrenade::ActionGrab()
{
	//LineTrace and see if we reach any actors with physics body collision channel set
	GetFirstPhysicsbodyInReach();

	SplinePath->SetHiddenInGame(false);
	
	//If we hit something, attach a physics handle
	auto HitResult = GetFirstPhysicsbodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();
	auto HasActorHit = HitResult.GetActor();

	if(HasActorHit)
	{
		//Grab line traced object
		PhysicsHandle->GrabComponent(ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation(), true);
		
		//Draw debug line
		FVector ObjectForwardvector = PlayerViewpointRotation.Vector();
		FPredictProjectilePathParams PredictParams;
		FVector OffsetLocation = {0,0,1};
		PredictParams.StartLocation = PlayerViewpointLocation;
		PredictParams.LaunchVelocity = ObjectForwardvector*2000;
		PredictParams.ProjectileRadius = 3000.0f;
		FPredictProjectilePathResult PredictResult;
		bool bHit = UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResult);
		
		TArray<FVector> PointLocation;
		for (auto Pathpoint : PredictResult.PathData)
		{
			PointLocation.Add(Pathpoint.Location);
		}
		
		SplinePath->SetSplinePoints(PointLocation, ESplineCoordinateSpace::World);
		SplinePath->SetDrawDebug(true);
		
		UE_LOG(LogTemp, Warning, TEXT("Predicted Flying Path"));
		
	}
}

//Dehighlight the object has been grabbed and throw it forward
void UExplodeGrenade::ActionRelease()
{
	SplinePath->SetHiddenInGame(true);
	if(PhysicsHandle->GrabbedComponent)
	{
		//Do release physics handle
		UE_LOG(LogTemp, Warning, TEXT("Grab Released"));
		//auto ObjectMaterial = PhysicsHandle->GrabbedComponent->GetMaterial(0);
		//auto DynamicMaterial = PhysicsHandle->GrabbedComponent->CreateDynamicMaterialInstance(0, ObjectMaterial);
		//DynamicMaterial->SetScalarParameterValue("Highlight", 0.0f);
		HoldComponent = PhysicsHandle->GrabbedComponent;
		PhysicsHandle->ReleaseComponent();

		//Add an impulse forward velocity when release the object
		FVector ObjectForwardvector = PlayerViewpointRotation.Vector();
		HoldComponent->AddImpulse(ObjectForwardvector*ThrowStrength);
		UE_LOG(LogTemp, Warning, TEXT("Throw Object"));

		//Trigger explosion, object fleshing then run explosion after 5 seconds 
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UExplodeGrenade::ExplodeAction, 1.0f, true, 0.0f);
		auto FlashingMaterial = HoldComponent->GetMaterial(0);
		auto FlashingDynamicMaterial = HoldComponent->CreateDynamicMaterialInstance(0, FlashingMaterial);
		FlashingDynamicMaterial->SetScalarParameterValue("Highlight", 5.0f);
	}
}

//line trace an find an object to grab
const FHitResult UExplodeGrenade::GetFirstPhysicsbodyInReach()
	{
		UE_LOG(LogTemp, Warning, TEXT("Grab Pressed"));
		FVector LineTraceEnd = PlayerViewpointLocation + (PlayerViewpointRotation.Vector()*SearchScale);

		//Line trace object
		FHitResult TargetHit;
		GetWorld()->LineTraceSingleByObjectType(TargetHit, PlayerViewpointLocation, LineTraceEnd, FCollisionObjectQueryParams(ECC_PhysicsBody));
	
		//Draw Debug line for visualize hit
		DrawDebugLine(GetWorld(), PlayerViewpointLocation, LineTraceEnd, FColor::Red, false, 0.f, 0.f, 10.f);

		if(TargetHit.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit actor is : %s"), *(TargetHit.GetActor()->GetName()));
		}
	
		//Return last hit actor
		return TargetHit;
	}

void UExplodeGrenade::ExplodeAction()
{
	//If counter count to zero, destroy actor and spawn explosion
	if(CountDownSeconds == 0)
	{
		FVector GrenadeLocation = HoldComponent->GetComponentLocation();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		CountDownSeconds = -1;
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Stop Counting"));
		HoldComponent->DestroyComponent();
		UGameplayStatics::SpawnEmitterAtLocation(this->GetWorld(), ExplosionParticle, GrenadeLocation);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Grenade Explode"));
	}

	//If counter hasn't count to zero, print current count down number
	else if(CountDownSeconds >= 0)
	{
		CountDownSeconds -= 1;
		FString DelayTimeString = FString::SanitizeFloat(CountDownSeconds);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, *DelayTimeString);	
	}
	else
	{
		
	}
	
}