// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ParkourShooter/ParkourShooterCharacter.h"

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
}

FNetworkPredictionData_Client* UCustomCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr) {
		UCustomCharacterMovementComponent* MutableThis = const_cast<UCustomCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Custom(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

bool UCustomCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UCustomCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UCustomCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UCustomCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	//UE_LOG(LogTemp, Warning, TEXT("CUSTOM MOVEMENT COMPONENT"));

	if (MovementMode == MOVE_Walking) {
		if (Safe_bWantsToSprint) {
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}

		else {
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UCustomCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PlayerCharacterOwner = Cast<AParkourShooterCharacter>(GetOwner());
}

void UCustomCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (MovementMode == MOVE_Walking && Safe_bWantsToSprint && bWantsToCrouch && !Safe_bPrevWantsToCrouch) {
		FHitResult PotentialSlideSurface;

		//UE_LOG(LogTemp, Warning, TEXT("%f %d %d"), Velocity.SizeSquared(), bWantsToCrouch, Safe_bPrevWantsToCrouch);

		UE_LOG(LogTemp, Warning, TEXT("%f %f %d"), Velocity.SizeSquared(), pow(MinSlideSpeed, 2), GetSlideSurface(PotentialSlideSurface));

		if (Velocity.SizeSquared() > pow(MinSlideSpeed, 2) && GetSlideSurface(PotentialSlideSurface)) {
			EnterSlide();
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide)) {
		ExitSlide();
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode) {
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
	}
}

void UCustomCharacterMovementComponent::SprintPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Sprinting"));
	Safe_bWantsToSprint = true;
}

void UCustomCharacterMovementComponent::SprintReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Not Sprinting"));
	Safe_bWantsToSprint = false;
}

void UCustomCharacterMovementComponent::CrouchPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Crouched"));
	bWantsToCrouch = ~bWantsToCrouch;
}

bool UCustomCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void UCustomCharacterMovementComponent::EnterSlide()
{
	UE_LOG(LogTemp, Warning, TEXT("Slide Entered"));
	//bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UCustomCharacterMovementComponent::ExitSlide()
{
	UE_LOG(LogTemp, Warning, TEXT("Slide Exited"));
	bWantsToCrouch = true;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
	SetMovementMode(MOVE_Walking);
}

void UCustomCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) {
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	FHitResult SurfaceHit;

	if (!GetSlideSurface(SurfaceHit) || Velocity.SizeSquared() < pow(MinSlideSpeed, 2)) {
		ExitSlide();
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	// Surface gravity
	Velocity += SlideGravityForce * FVector::DownVector * deltaTime;

	// Strafe
	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > 0.5) {
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}

	else {
		Acceleration = FVector::ZeroVector;
	}

	// Calc Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(deltaTime, SlideFrictionFactor, true, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(deltaTime);

	// Perform Move
	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, SurfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	FHitResult NewSurfaceHit;

	if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(MinSlideSpeed, 2)) {
		ExitSlide();
	}

	// Update outgoing velocity and acceleration
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

bool UCustomCharacterMovementComponent::GetSlideSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + PlayerCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, PlayerCharacterOwner->GetIgnoreCharacterParams());
}

UCustomCharacterMovementComponent::FSavedMove_Custom::FSavedMove_Custom()
{
	Saved_bWantsToSprint = 0;
	Saved_bWantsToProne = 0;
	Saved_bPrevWantsToCrouch = 0;
}

bool UCustomCharacterMovementComponent::FSavedMove_Custom::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Custom* NewCustomMove = static_cast<FSavedMove_Custom*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewCustomMove->Saved_bWantsToSprint) {
		return false;
	}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UCustomCharacterMovementComponent::FSavedMove_Custom::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint) Result |= FLAG_Custom_0;

	return Result;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
}

UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Custom());
}
