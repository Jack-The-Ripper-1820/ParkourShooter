// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_Prone UMETA(DisplayName = "Prone"),
	CMOVE_Climb	UMETA(DisplayName = "Climb"),
	CMOVE_MAX UMETA(Hidden),
};


UCLASS()
class PARKOURSHOOTER_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
		class FSavedMove_Custom : public FSavedMove_Character {
		public:

			enum CompressedFlags
			{
				FLAG_Sprint = 0x10,
				FLAG_Dash = 0x20,
				FLAG_Custom_2 = 0x40,
				FLAG_Custom_3 = 0x80,
			};

			typedef FSavedMove_Character Super;

			uint8 Saved_bWantsToSprint : 1;

			uint8 Saved_bPrevWantsToCrouch : 1;

			uint8 Saved_bWantsToProne : 1;

		public:
			FSavedMove_Custom();


			virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
			virtual void Clear();
			virtual uint8 GetCompressedFlags() const override;
			virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
			virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Custom : public FNetworkPredictionData_Client_Character {
	public:
		FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	// Sprint
	UPROPERTY(EditDefaultsOnly) float MaxSprintSpeed = 750;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxWalkSpeed = 500;


	// Slide
	UPROPERTY(EditDefaultsOnly) float MinSlideSpeed = 400;
	UPROPERTY(EditDefaultsOnly) float MaxSlideSpeed = 400;
	UPROPERTY(EditDefaultsOnly) float SlideEnterImpulse = 400.f;
	UPROPERTY(EditDefaultsOnly) float SlideGravityForce = 4000.f;
	UPROPERTY(EditDefaultsOnly) float SlideFrictionFactor = 0.06f;
	UPROPERTY(EditDefaultsOnly) float BrakingDecelerationSliding = 1000.f;

	// Prone
	UPROPERTY(EditDefaultsOnly) float ProneEnterHoldDuration = .2f;
	UPROPERTY(EditDefaultsOnly) float ProneSlideEnterImpulse = 300.f;
	UPROPERTY(EditDefaultsOnly) float MaxProneSpeed = 300.f;
	UPROPERTY(EditDefaultsOnly) float BrakingDecelerationProning = 2500.f;

	UPROPERTY(Transient) class AParkourShooterCharacter* PlayerCharacterOwner;

	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;
	bool Safe_bWantsToProne;
	FTimerHandle TimerHandle_EnterProne;

public:
	UCustomCharacterMovementComponent();

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void InitializeComponent() override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();
	UFUNCTION(BlueprintCallable) void PronePressed();
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;


	// Slide
private:
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool CanSlide() const;

	// Prone
private:
	FORCEINLINE void TryEnterProne() { Safe_bWantsToProne = true; }
	UFUNCTION(Server, Reliable) void Server_EnterProne();

	void EnterProne(EMovementMode PrevMode, ECustomMovementMode PrevCustomMove);
	void ExitProne();
	bool CanProne() const;
	void PhysProne(float deltaTime, int32 Iterations);

	// Climb
private:
	bool TryClimb();
	void PhysClimb(float deltaTime, int32 Iterations);
};
