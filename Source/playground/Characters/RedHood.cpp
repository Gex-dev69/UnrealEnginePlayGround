// Copyright Epic Games, Inc. All Rights Reserved.

#include "RedHood.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ARedHood

UAudioComponent* AudioComponent;
FVector CharacterVelocity;
FTimerHandle MusicTimer;

ARedHood::ARedHood()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

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
	GetCharacterMovement()->MaxWalkSpeed = 120.f;
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
void ARedHood::BeginPlay()
{
	Super::BeginPlay();
	ControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AudioComponent = UGameplayStatics::SpawnSoundAttached(music, GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true, true);
	AudioComponent->SetPaused(true);
	GetWorld()->GetTimerManager().SetTimer(MusicTimer, this, &ARedHood::RockThemeSong, 0.5f, true);
}

void ARedHood::RockThemeSong()
{
	CharacterVelocity = GetVelocity();
	float Movement = CharacterVelocity.SizeSquared();
	FString Message = FString::Printf(TEXT("Velocity: %.6f"), Movement);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Message);
	if (Movement > 0.0f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Moving");
		
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Playing Audio");
			AudioComponent->SetPaused(false);
		
	}
	else if(Movement <= 0.0f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Not Moving");

		if (AudioComponent->IsPlaying())
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Not Playing Audio");
			AudioComponent->SetPaused(true);
		}
	}
}

void ARedHood::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ControllerRef != nullptr && ControllerRef->WasInputKeyJustPressed(EKeys::E))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("From Red Hood"));
		PlayAnimMontage(callMe);
	}

	
}

void ARedHood::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ARedHood::StartRunning);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ARedHood::StopRunning);
	
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ARedHood::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ARedHood::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ARedHood::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ARedHood::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARedHood::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARedHood::TouchStopped);
}

void ARedHood::StartRunning()
{
	for(float i = 0;i < 500.0;i++)
	{
		GetCharacterMovement()->MaxWalkSpeed = i;
	}
	
}

void ARedHood::StopRunning()
{
	// Use for loop maybe to slowly decrease the speed instead of BLAK into Walk animation
	GetCharacterMovement()->MaxWalkSpeed = 120.0f;
}
void ARedHood::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ARedHood::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ARedHood::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ARedHood::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ARedHood::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARedHood::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
