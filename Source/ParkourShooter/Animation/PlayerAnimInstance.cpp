// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "ParkourShooter/ParkourShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ParkourShooter/Weapon/Weapon.h"

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

	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

	bIsCrouched = PlayerCharacter->GetCharacterMovement()->IsCrouching();

	bAiming = PlayerCharacter->IsAiming();

	bFirstPerson = PlayerCharacter->bFirstPerson;

	EquippedWeapon = PlayerCharacter->GetEquippedWeapon();

	// Offset Yaw for Strafing
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / Deltatime;
	const float Interp = FMath::FInterpTo(Lean, Target, Deltatime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = PlayerCharacter->GetAO_Yaw();
	AO_Pitch = PlayerCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh()) {
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandPlacement"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PlayerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}

}
	
