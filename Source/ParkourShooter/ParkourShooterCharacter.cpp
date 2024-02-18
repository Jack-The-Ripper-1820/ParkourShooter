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
#include "Net/UnrealNetwork.h"
#include "ParkourShooter/Weapon/Weapon.h"
#include "ParkourShooter/Components/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ParkourShooter/Animation/PlayerAnimInstance.h"


//////////////////////////////////////////////////////////////////////////
// AParkourShooterCharacter

AParkourShooterCharacter::AParkourShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	CustomCharacterMovement = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());
	CustomCharacterMovement->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	/*bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;*/

	bUseControllerRotationYaw = true;

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
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	ThirdPersonCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	ThirdPersonCamera->Deactivate();
	FirstPersonCamera->Activate();
	bFirstPerson = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AParkourShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
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


void AParkourShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon) {
		LastWeapon->ShowPickupWidget(false);
	}
}

void AParkourShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AParkourShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat) {
		Combat->Character = this;
	}

}

void AParkourShooterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

bool AParkourShooterCharacter::IsWeaponEquipped()
{
	return Combat && Combat->EquippedWeapon;
}

bool AParkourShooterCharacter::IsAiming()
{
	return Combat && Combat->bAiming;
}

AWeapon* AParkourShooterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
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

void AParkourShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AParkourShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
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

		//Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::EquipPressed);

		//AimPressed
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AParkourShooterCharacter::AimPressed);

		//AimReleased
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::AimReleased);

		//FirePressed
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AParkourShooterCharacter::FirePressed);

		//FireReleased
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AParkourShooterCharacter::FireReleased);
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
	UE_LOG(LogTemp, Warning, TEXT("Switch Camera"));
	if (ThirdPersonCamera && ThirdPersonCamera->IsActive()) {
		ThirdPersonCamera->Deactivate();
		FirstPersonCamera->Activate();
		bUseControllerRotationYaw = true;
		bFirstPerson = true;
		/*if (IsLocallyControlled()) {
			GetMesh()->HideBoneByName(TEXT("head"), PBO_None);
		}*/
	}

	else if ((FirstPersonCamera && FirstPersonCamera->IsActive())) {
		FirstPersonCamera->Deactivate();
		ThirdPersonCamera->Activate();
		bFirstPerson = false;
		bUseControllerRotationYaw = true;
		/*if (IsLocallyControlled()) {
			GetMesh()->UnHideBoneByName(TEXT("head"));
		}*/
	}
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

void AParkourShooterCharacter::EquipPressed(const FInputActionValue& Value)
{
	if (Combat) {
		if (HasAuthority()) {
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else {
			ServerEquipPressed();
		}
	}
}

void AParkourShooterCharacter::AimPressed(const FInputActionValue& Value)
{
	if (Combat) {
		Combat->SetAiming(true);
	}
}

void AParkourShooterCharacter::AimReleased(const FInputActionValue& Value)
{
	if (Combat) {
		Combat->SetAiming(false);
	}
}

void AParkourShooterCharacter::FirePressed(const FInputActionValue& Value)
{
	if (Combat) {
		Combat->FireButtonPressed(true);
	}
}

void AParkourShooterCharacter::FireReleased(const FInputActionValue& Value)
{
	if (Combat) {
		Combat->FireButtonPressed(false);
	}
}

void AParkourShooterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) { // standing still, not jumping 
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;
	}

	if (Speed > 0.f || bIsInAir) {// running, or jumping
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled()) {
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AParkourShooterCharacter::ServerEquipPressed_Implementation() {
	if (Combat) {
		Combat->EquipWeapon(OverlappingWeapon);
	}
}




