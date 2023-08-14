// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkourShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ParkourShooter/Components/CustomCharacterMovementComponent.h"



//////////////////////////////////////////////////////////////////////////
// AParkourShooterCharacter

AParkourShooterCharacter::AParkourShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	CustomCharacterMovement = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());
	CustomCharacterMovement->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AParkourShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	/*if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {

		UE_LOG(LogTemp, Warning, TEXT("PLayerController Casted"));

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {

			UE_LOG(LogTemp, Warning, TEXT("Subsystem Casted"));


			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}*/
}

FCollisionQueryParams AParkourShooterCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

void AParkourShooterCharacter::Jump()
{
	Super::Jump();

	UE_LOG(LogTemp, Warning, TEXT("Pressed Jump: %d"), bPressedJump);

	bPressedCustomJump = true;
	bPressedJump = false;
}

void AParkourShooterCharacter::StopJumping()
{
	Super::StopJumping();

	bPressedCustomJump = false;
}

void AParkourShooterCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AParkourShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AParkourShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AParkourShooterCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AParkourShooterCharacter::Look);

		//Switch Camera
		EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::SwitchCamera);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::Sprint);

		//CrouchPressed
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AParkourShooterCharacter::CrouchPressed);

		//CrouchReleased
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::CrouchReleased);


		//DashPressed
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AParkourShooterCharacter::DashPressed);

		//DashReleased
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::DashReleased);
	}

}

void AParkourShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AParkourShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AParkourShooterCharacter::SwitchCamera(const FInputActionValue& Value)
{
}

void AParkourShooterCharacter::Sprint(const FInputActionValue& Value)
{
	if (bIsSprinting) {
		bIsSprinting = false;
		CustomCharacterMovement->SprintReleased();
	}
	else {
		bIsSprinting = true;
		CustomCharacterMovement->SprintPressed();
	}
}

void AParkourShooterCharacter::CrouchPressed(const FInputActionValue& Value)
{
	//bIsCrouched = ~bIsCrouched;
	CustomCharacterMovement->CrouchPressed();
}

void AParkourShooterCharacter::CrouchReleased(const FInputActionValue& Value)
{
	//bIsCrouched = false;
	CustomCharacterMovement->CrouchReleased();
}

void AParkourShooterCharacter::DashPressed(const FInputActionValue& Value)
{
	CustomCharacterMovement->DashPressed();
}

void AParkourShooterCharacter::DashReleased(const FInputActionValue& Value)
{
	CustomCharacterMovement->DashReleased();
}




