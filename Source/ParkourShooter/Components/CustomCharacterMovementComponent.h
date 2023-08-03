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
	CMOVE_MAX UMETA(Hidden),
};


UCLASS()
class PARKOURSHOOTER_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
		class FSavedMove_Custom : public FSavedMove_Character {
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

	UPROPERTY(EditDefaultsOnly) float Sprint_MaxWalkSpeed = 750;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxWalkSpeed = 500;

	UPROPERTY(EditDefaultsOnly) float MinSlideSpeed = 350;
	UPROPERTY(EditDefaultsOnly) float MaxSlideSpeed = 350;
	UPROPERTY(EditDefaultsOnly) float SlideEnterImpulse = 5000.f;
	UPROPERTY(EditDefaultsOnly) float SlideGravityForce = 5000.f;
	UPROPERTY(EditDefaultsOnly) float SlideFrictionFactor = 0.3f;
	UPROPERTY(EditDefaultsOnly) float BrakingDecelerationSliding = 1000.f;

	UPROPERTY(Transient) class AParkourShooterCharacter* PlayerCharacterOwner;

	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;


public:
	UCustomCharacterMovementComponent();

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void InitializeComponent() override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

private:
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;
};
