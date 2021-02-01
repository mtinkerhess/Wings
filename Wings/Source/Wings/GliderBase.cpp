#include "GliderBase.h"

#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"

namespace
{
	float SlewToward(float current, float target, float min, float max, UCurveFloat* slewAmountByAmountToLimitCurve, float deltaTime)
	{
		float amountToLimit = target > current ? (max - current) : (current - min);
		float slew = slewAmountByAmountToLimitCurve->GetFloatValue(amountToLimit);
		return FMath::Lerp(current, target, slew * deltaTime);
	}

	float SlewToward(float current, float target, UCurveFloat* slewAmountByAmountToTargetCurve, float deltaTime)
	{
		float amountToTarget = FMath::Abs(target - current);
		float slew = slewAmountByAmountToTargetCurve->GetFloatValue(amountToTarget);
		return FMath::Lerp(current, target, slew * deltaTime);
	}
}

// Sets default values
AGliderBase::AGliderBase()
{
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> GliderMesh;
		FConstructorStatics()
			: GliderMesh(TEXT("StaticMesh'/Game/Models/Glider/Glider.Glider'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GliderMesh0"));
	GliderMesh->SetStaticMesh(ConstructorStatics.GliderMesh.Get());	// Set static mesh
	RootComponent = GliderMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f, 0.f, 10.f);
	SpringArm->bEnableCameraLag = false;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
}

void AGliderBase::NotifyThermalBeginOverlap(AActor* thermal)
{
	OverlappingThermals.Add(thermal);
}

void AGliderBase::NotifyThermalEndOverlap(AActor* thermal)
{
	OverlappingThermals.Remove(thermal);
}

// Called when the game starts or when spawned
void AGliderBase::BeginPlay()
{
	Super::BeginPlay();
}

namespace	
{
	float SignedSquare(float f)
	{
		if (f < 0.0f)
		{
			return -(f * f);
		}
		else
		{
			return f * f;
		}
	}
}

// Called every frame
void AGliderBase::Tick(float DeltaTime)
{
	/*
	
	Always move in local forward vector.

	ROLL

	Neutral roll is always 0.

	Absolute roll limits.

	Roll input slew is a function of:
	- User input on / off (user input -> faster, back to neutral -> slower)
	- How close to limit (closer to limit -> slower)

	Adjust roll based on slewed user input.

	Delta roll is a function of:

	- Slewed user input
	- Current roll

	ROLL STATE

	TargetRoll
	Roll

	ROLL TUNING VARIABLES

	MaxRoll
	RollInputSlewByAmountToLimitCurve
	RollNoInputSlew

	YAW

	Delta yaw is a function of:

	- Roll
	
	YAW TUNING VARIABLES

	DeltaYawByRollCurve

	VELOCITY

	Adjust target velocity based on:

	- Pitch (pitch down -> higher velocity)
	- Roll (more roll -> lower velocity)

	Delta velocity is a function of:

	- Target velocity

	VELOCITY STATE

	TargetVelocity
	Velocity

	VELOCITY TUNING VARIABLES

	TargetVelocityByPitchCurve
	TargetVelocityModifierByRollCurve
	VelocitySlewByTargetVelocityAcceleratingCurve
	VelocitySlewByTargetVelocityDeceleratingCurve

	*/

	Super::Tick(DeltaTime);

	///////////
	// PITCH //
	///////////

	// Adjust neutral pitch based on:
	// - Velocity (slow velocity -> neutral pitch down)
	// - Roll (more roll -> neutral pitch down)
	NeutralPitch = NeutralPitchByVelocityCurve->GetFloatValue(Velocity) + NeutralPitchByRollCurve->GetFloatValue(FMath::Abs(Roll));

	// Adjust pitch limits based on neutral pitch and absolute limits.
	float maxPitch = MaxPitchByVelocityCurve->GetFloatValue(Velocity);

	float inputPitch = UpInputAxis < 0.0f ? FMath::Lerp(NeutralPitch, maxPitch, -UpInputAxis) : FMath::Lerp(NeutralPitch, MinPitch, UpInputAxis);

	// Pitch input slew amount is a function of:
	// - User input on / off (user input -> faster, back to neutral -> slower)
	// - How close to limit (closer to limit -> slower)
	if (FMath::IsNearlyZero(UpInputAxis))
	{
		SlewedInputPitch = FMath::Lerp(SlewedInputPitch, inputPitch, PitchNoInputSlew * DeltaTime);
	}
	else
	{
		SlewedInputPitch = SlewToward(SlewedInputPitch, inputPitch, MinPitch, maxPitch, PitchInputSlewByAmountToLimitCurve, DeltaTime);
	}

	// Adjust pitch based on slewed user input.
	// Delta pitch is a function of:
	// - Slewed user input
	// - Current pitch (distance from limits, slow down when approaching limits)
	Pitch = SlewToward(Pitch, SlewedInputPitch, MinPitch, maxPitch, PitchSlewByAmountToLimitCurve, DeltaTime);

	//////////
	// ROLL //
	//////////

	// Roll input slew is a function of:
	// - User input on / off (user input -> faster, back to neutral -> slower)
	// - How close to limit (closer to limit -> slower)

	float inputRoll = RightInputAxis * MaxRoll;

	if (FMath::IsNearlyZero(inputRoll))
	{
		SlewedInputRoll = FMath::Lerp(SlewedInputRoll, inputRoll, RollNoInputSlew * DeltaTime);
	}
	else
	{
		SlewedInputRoll = SlewToward(SlewedInputRoll, inputRoll, -MaxRoll, MaxRoll, RollInputSlewByAmountToLimitCurve, DeltaTime);
	}

	// Adjust roll based on slewed user input.
	// Delta roll is a function of:
	// - Slewed user input
	// - Current roll

	Roll = SlewToward(Roll, SlewedInputRoll, -MaxRoll, MaxRoll, RollSlewByAmountToLimitCurve, DeltaTime);

	/////////
	// YAW //
	/////////

	float deltaYaw = Roll >= 0.0f ? DeltaYawByRollCurve->GetFloatValue(Roll) : -DeltaYawByRollCurve->GetFloatValue(-Roll);
	Yaw = Yaw + deltaYaw * DeltaTime + 360.0f;
	if (Yaw > 360.0f) Yaw -= 360.0f;

	//////////////
	// Velocity //
	//////////////

	// Adjust target velocity based on:
	// - Pitch (pitch down -> higher velocity)
	// - Roll (more roll -> lower velocity)
	float sinPitch = FMath::Sin(FMath::DegreesToRadians(Pitch));
	float verticalVelocity = sinPitch * Velocity;
	Velocity += DeltaVelocityByVerticalVelocityCurve->GetFloatValue(verticalVelocity) * DeltaTime; // *TargetVelocityModifierByRollCurve->GetFloatValue(FMath::Abs(Roll));

	Velocity -= SignedSquare(sinPitch) * Gravity * DeltaTime;

	Velocity = FMath::Clamp(Velocity, 0.01f, 1.0f);

	//////////////
	// Thermals //
	//////////////

	if (OverlappingThermals.Num() > 0)
	{
		ThermalLift = FMath::Min(ThermalLiftMaxVelocity, ThermalLift + ThermalLiftAcceleration * DeltaTime);
	}
	else
	{
		ThermalLift = FMath::Max(0.0f, ThermalLift - ThermalLiftAcceleration * DeltaTime);
	}

	//////////////////
	// Update actor //
	//////////////////

	SetActorRotation(FRotator(Pitch, Yaw, Roll));
	AddActorLocalOffset(FVector(Velocity * VelocityScale * DeltaTime, 0.0f, 0.0f));
	AddActorWorldOffset(FVector(0.0f, 0.0f, ThermalLift * DeltaTime));

	//////////////////
	// Debug String //
	//////////////////

	DebugStateString = FString::Printf(
		TEXT(
			"Velocity: %f"
			"\nPitch: %f"),
		Velocity,
		Pitch);
}

void AGliderBase::SetupPlayerInputComponent(class UInputComponent* playerInputComponent)
{
	// Check if PlayerInputComponent is valid (not NULL)
	check(playerInputComponent);

	// Bind our control axis' to callback functions
	playerInputComponent->BindAxis("MoveUp", this, &AGliderBase::MoveUpInput);
	playerInputComponent->BindAxis("MoveRight", this, &AGliderBase::MoveRightInput);
}

void AGliderBase::MoveUpInput(float Val)
{
	UpInputAxis = FMath::Clamp(Val, -1.0f, 1.0f);
}

void AGliderBase::MoveRightInput(float Val)
{
	RightInputAxis = FMath::Clamp(Val, -1.0f, 1.0f);
}
