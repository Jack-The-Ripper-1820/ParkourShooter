// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent()
{
}

FNetworkPredictionData_Client* UCustomCharacterMovementComponent::GetPredictionData_Client() const
{
	return nullptr;
}

bool UCustomCharacterMovementComponent::IsMovingOnGround() const
{
	return false;
}

bool UCustomCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return false;
}

void UCustomCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
}

void UCustomCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
}

void UCustomCharacterMovementComponent::InitializeComponent()
{
}

void UCustomCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
}

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
}

void UCustomCharacterMovementComponent::SprintPressed()
{
}

void UCustomCharacterMovementComponent::SprintReleased()
{
}

void UCustomCharacterMovementComponent::CrouchPressed()
{
}

bool UCustomCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return false;
}

void UCustomCharacterMovementComponent::EnterSlide()
{
}

void UCustomCharacterMovementComponent::ExitSlide()
{
}

void UCustomCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
}

bool UCustomCharacterMovementComponent::GetSlideSurface(FHitResult& Hit) const
{
	return false;
}

UCustomCharacterMovementComponent::FSavedMove_Custom::FSavedMove_Custom()
{
}

bool UCustomCharacterMovementComponent::FSavedMove_Custom::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	return false;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::Clear()
{
}

uint8 UCustomCharacterMovementComponent::FSavedMove_Custom::GetCompressedFlags() const
{
	return uint8();
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::PrepMoveFor(ACharacter* C)
{
}

UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::AllocateNewMove()
{
	return FSavedMovePtr();
}
