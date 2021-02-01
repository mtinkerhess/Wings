#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "GameFramework/Actor.h"
#include "GliderBase.generated.h"

UCLASS()
class WINGS_API AGliderBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGliderBase();

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* GliderMesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	// Pitch settings

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		UCurveFloat* MaxPitchByVelocityCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		UCurveFloat* NeutralPitchByVelocityCurve;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		UCurveFloat* NeutralPitchByRollCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		float MinPitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		float MaxPitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		float MaxPitchAboveNeutral;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		float MinPitchBelowNeutral;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		UCurveFloat* PitchInputSlewByAmountToLimitCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		float PitchNoInputSlew;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Pitch")
		UCurveFloat* PitchSlewByAmountToLimitCurve;

	// Roll settings

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Roll")
		float MaxRoll;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Roll")
		UCurveFloat* RollInputSlewByAmountToLimitCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Roll")
		float RollNoInputSlew;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Roll")
		UCurveFloat* RollSlewByAmountToLimitCurve;

	// Yaw settings

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Roll")
		UCurveFloat* DeltaYawByRollCurve;

	// Velocity settings

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Velocity")
		float VelocityScale = 3500.0f; // 80 mph in cm/s

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Velocity")
		UCurveFloat* DeltaVelocityByVerticalVelocityCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Velocity")
		float Gravity;

	// Other settings

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Other")
		float ThermalLiftMaxVelocity = 500.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|Settings|Other")
		float ThermalLiftAcceleration = 1000.0f;

	// State

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Glider|State")
		FString DebugStateString;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Glider|State")
		TArray<AActor*> OverlappingThermals;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Glider|State")
		float ThermalLift = 0.0f;

	// Functions

	UFUNCTION(BlueprintCallable, Category = "Glider")
		void NotifyThermalBeginOverlap(AActor* thermal);

	UFUNCTION(BlueprintCallable, Category = "Glider")
		void NotifyThermalEndOverlap(AActor* thermal);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides

	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

private:

	float NeutralPitch = 0.0f;
	float SlewedInputPitch = 0.0f;
	float Pitch = 0.0f;

	float SlewedInputRoll = 0.0f;
	float Roll = 0.0f;

	float Yaw = 0.0f;

	float TargetVelocity = 0.0f;
	float Velocity = 0.0f;

	float RightInputAxis = 0.0f;
	float UpInputAxis = 0.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
