// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDashStartDelegate);

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_Prone UMETA(DisplayName = "Prone"),
	CMOVE_Dash UMETA(DisplayName = "Dash"),
	CMOVE_WallRun UMETA(DisplayName = "WallRun"),
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

		uint8 Saved_bWantsToDash : 1;

		uint8 Saved_bPressedCustomJump : 1;

		uint8 Saved_bHadAnimRootMotion : 1;

		uint8 Saved_bTransitionFinished : 1;

		uint8 Saved_bWallRunIsRight : 1;


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

	// Dash
	UPROPERTY(EditDefaultsOnly) float DashCooldownDuration = 2.f;
	UPROPERTY(EditDefaultsOnly) float AuthDashCooldownDuration = .9f;
	UPROPERTY(EditDefaultsOnly) float DashImpulse = 2000.f;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* DashMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* SlideMontage;

	// Mantle
	UPROPERTY(EditDefaultsOnly) float MantleMaxDistance = 200;
	UPROPERTY(EditDefaultsOnly) float MantleReachHeight = 50;
	UPROPERTY(EditDefaultsOnly) float MinMantleDepth = 30;
	UPROPERTY(EditDefaultsOnly) float MantleMinWallSteepnessAngle = 75;
	UPROPERTY(EditDefaultsOnly) float MantleMaxSurfaceAngle = 40;
	UPROPERTY(EditDefaultsOnly) float MantleMaxAlignmentAngle = 45;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionTallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ProxyTallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ShortMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionShortMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ProxyShortMantleMontage;

	// Wall Run
	UPROPERTY(EditDefaultsOnly) float MinWallRunSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly) float MaxWallRunSpeed = 800.f;
	UPROPERTY(EditDefaultsOnly) float MaxVerticalWallRunSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly) float WallRunDisengageAngle = 75;
	UPROPERTY(EditDefaultsOnly) float WallMagnetismForce = 200.f;
	UPROPERTY(EditDefaultsOnly) float MinWallRunHeight = 50.f;
	UPROPERTY(EditDefaultsOnly) UCurveFloat* WallRunGravityScaleCurve;
	UPROPERTY(EditDefaultsOnly) float WallEjectForce = 300.f;

	UPROPERTY(Transient) class AParkourShooterCharacter* PlayerCharacterOwner;

	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;
	bool Safe_bWantsToProne;
	bool Safe_bWantsToDash;

	bool Safe_bHadAnimRootMotion;
	bool Safe_bTransitionFinished;

	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;

	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;

	FString TransitionName;

	float TransitionQueuedMontageSpeed;
	int TransitionRMS_ID;

	float DashStartTime;

	FTimerHandle TimerHandle_EnterProne;
	FTimerHandle TimerHandle_DashCooldown;

	bool Safe_bWallRunIsRight;

	// Replication
	UPROPERTY(ReplicatedUsing = OnRep_Dash) bool Proxy_bDash;

	UPROPERTY(ReplicatedUsing = OnRep_ShortMantle) bool Proxy_bShortMantle;
	UPROPERTY(ReplicatedUsing = OnRep_TallMantle) bool Proxy_bTallMantle;

	// Delegates
public:
	UPROPERTY(BlueprintAssignable) FDashStartDelegate DashStartDelegate;

public:
	UCustomCharacterMovementComponent();

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void InitializeComponent() override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();
	UFUNCTION(BlueprintCallable) void DashPressed();
	UFUNCTION(BlueprintCallable) void DashReleased();
	UFUNCTION(BlueprintCallable) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintCallable) bool IsMovementMode(EMovementMode InMovementMode) const;
	UFUNCTION(BlueprintCallable) FORCEINLINE bool IsWallRunning() const { return IsCustomMovementMode(CMOVE_WallRun); }
	UFUNCTION(BlueprintCallable) FORCEINLINE bool WallRunningIsRight() const { return Safe_bWallRunIsRight; }


	// Slide
private:
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool CanSlide() const;

	// Prone
private:
	void TryEnterProne();
	UFUNCTION(Server, Reliable) void Server_EnterProne();

	void EnterProne(EMovementMode PrevMode, ECustomMovementMode PrevCustomMove);
	void ExitProne();
	bool CanProne() const;
	void PhysProne(float deltaTime, int32 Iterations);

	// Climb
private:
	bool TryClimb();
	void PhysClimb(float deltaTime, int32 Iterations);

	// Dash
private:
	void OnDashCooldownFinished();

	bool CanDash() const;
	void PerformDash();

	// Mantle
private:
	bool TryMantle();
	FVector GetMantleStartLocation(FHitResult FromHit, FHitResult SurfaceHit, bool bTallMantle) const;

	// Wall Run
private:
	bool TryWallRun();
	void PhysWallRun(float deltaTime, int32 Iterations);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION() void OnRep_Dash();
	UFUNCTION() void OnRep_ShortMantle();
	UFUNCTION() void OnRep_TallMantle();

public:
	bool IsServer() const;
	float GetScaledCapsuleRadius() const;
	float GetScaledCapsuleHalfHeight() const;

};
