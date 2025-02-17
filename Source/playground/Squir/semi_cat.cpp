// Fill out your copyright notice in the Description page of Project Settings.


#include "semi_cat.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Animation/AnimMontage.h"



float vv = 1.0f;
FTimerHandle visibilityTimerHandler;

// Sets default values
Asemi_cat::Asemi_cat()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


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
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called when the game starts or when spawned
void Asemi_cat::BeginPlay()
{
	Super::BeginPlay();
	controllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &Asemi_cat::OnMontageEnded);
	}
	MyMaterialInstance = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	


}



// Called every frame
void Asemi_cat::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);	
	
	// if (controllerRef != NULL)
	// {
	// 	if (controllerRef->WasInputKeyJustPressed(EKeys::E))
	// 	{
	// 		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Key E as pressed"));
	// 		PlayAnimMontage(callMe);
	// 		GetWorld()->SpawnActor<AActor>(SpawnYoBoi, GetActorLocation(), GetActorRotation());
	// 	}
	// }
}



// Called to bind functionality to input
void Asemi_cat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{

	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Action 1", IE_Pressed, this, &Asemi_cat::SpawnSlaver);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &Asemi_cat::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &Asemi_cat::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &Asemi_cat::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &Asemi_cat::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &Asemi_cat::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &Asemi_cat::TouchStopped);
	
}


void Asemi_cat::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void Asemi_cat::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void Asemi_cat::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void Asemi_cat::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void Asemi_cat::MoveForward(float Value)
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

void Asemi_cat::MoveRight(float Value)
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

void Asemi_cat::SpawnSlaver()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Key E as pressed"));
	PlayAnimMontage(callMe);
	
}

void Asemi_cat::DelayedFunction()
{
	vv = vv - 0.1f;
	float Visibility = MyMaterialInstance->K2_GetScalarParameterValue("Appearance");
	Visibility = Visibility - 0.04f; 
	MyMaterialInstance->SetScalarParameterValue("Appearance", Visibility);
	
	FString Message = FString::Printf(TEXT("Visibility: %.6f"), Visibility);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, Message);
	if (Visibility <= 0.0f)
    {
        // Condition to stop the loop
        GetWorld()->GetTimerManager().ClearTimer(visibilityTimerHandler);
    }
}
void Asemi_cat::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/*if (Montage == callMe)
	{
		GetWorld()->SpawnActor<AActor>(SpawnYoBoi, GetActorLocation(), GetActorRotation());
	}*/
	if (MyMaterialInstance)
	{
		float Delay = 0.5f;  // Adjust the delay time in seconds
		GetWorld()->GetTimerManager().SetTimer(visibilityTimerHandler, this, &Asemi_cat::DelayedFunction, Delay, true);
	}
}
