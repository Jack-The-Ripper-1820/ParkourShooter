// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "ParkourShooter/Components/CustomCharacterMovementComponent.h"
#include "ParkourShooter/ParkourShooterCharacter.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	PlayerCharacter = Cast<AParkourShooterCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float Deltatime)
{
	Super::NativeUpdateAnimation(Deltatime);

	if (PlayerCharacter == nullptr) {
		PlayerCharacter = Cast<AParkourShooterCharacter>(TryGetPawnOwner());
	}

	if (PlayerCharacter == nullptr) return;

	bIsCrouched = PlayerCharacter->bIsCrouched;

	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsProne = PlayerCharacter->GetCustomCharacterMovement()->IsMovementMode(MOVE_Custom) && PlayerCharacter->GetCustomCharacterMovement()->IsCustomMovementMode(CMOVE_Prone);

	bIsSliding = PlayerCharacter->GetCustomCharacterMovement()->IsMovementMode(MOVE_Custom) && PlayerCharacter->GetCustomCharacterMovement()->IsCustomMovementMode(CMOVE_Slide);
	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();
	bIsWallRunning = PlayerCharacter->GetCustomCharacterMovement()->IsWallRunning();

}
