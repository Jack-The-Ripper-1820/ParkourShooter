// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ParkourShooterCharacter.generated.h"


UCLASS(config = Game)
class AParkourShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UCustomCharacterMovementComponent* CustomCharacterMovement;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ThirdPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

public:
	AParkourShooterCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;

	FCollisionQueryParams GetIgnoreCharacterParams() const;

	bool bIsSprinting = false;
	//bool bIsCrouched = false;
	bool bIsProne = false;
	bool bPressedCustomJump;
	bool bIsDashing = false;

	virtual void Jump() override;
	virtual void StopJumping() override;

	bool bFirstPerson = true;


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void SwitchCamera(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	void CrouchPressed(const FInputActionValue& Value);
	void CrouchReleased(const FInputActionValue& Value);
	void DashPressed(const FInputActionValue& Value);
	void DashReleased(const FInputActionValue& Value);



protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return ThirdPersonCamera; }

	/** Returns Cusotm Character Movement Component subobject **/
	FORCEINLINE class UCustomCharacterMovementComponent* GetCustomCharacterMovement() const { return CustomCharacterMovement; }
};

