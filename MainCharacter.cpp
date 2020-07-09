// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MeleeWeapon.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "RangedWeapon.h"
#include "Runtime/Engine/Public/TimerManager.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera Boom - Pulls towards the player if there's a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;
	GetCapsuleComponent()->SetCapsuleHalfHeight(93.0f);
	GetCapsuleComponent()->SetCapsuleRadius(32.0f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	BurstParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BurstParticle"));

	// Set our turn rates for inputs
	BaseTurnRate = 65.0f;
	BaseLookUpRate = 65.0f;

	BaseWalkSpeed = 600.0f;
	BaseRunSpeed = 1000.0f;

	DashDistance = 10000.0f;
	DashCooldown = 1.0f;
	CanDash = true;
	DashStop = 0.1f;
	DashDelay = 0.2f;
	IsDashing = false;

	InCombat = true;
	IsAttacking = false;
	CurrentComboStatus = EComboStatus::ECS_Opener;
	CurrentBusterLevel = 0;


	static ConstructorHelpers::FObjectFinder<UStaticMesh>SwordMeshAsset(TEXT("StaticMesh'/Game/Character/Weapons/Katana/Katana.Katana'"));
	if (SwordMeshAsset.Succeeded())
	{
		SwordMesh = SwordMeshAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh>GunMeshAsset(TEXT("StaticMesh'/Game/Character/Weapons/Gun.Gun'"));
	if (GunMeshAsset.Succeeded())
	{
		GunMesh = GunMeshAsset.Object;
	}

	// Don't rotate when the controller rotates
	// It just affects the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure Character Movement
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of the input-
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // -at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 1000.0f;
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->CrouchedHalfHeight = 93.0f;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	AMeleeWeapon* SwordToEquip = GetWorld()->SpawnActor<AMeleeWeapon>();
	ARangedWeapon* GunToEquip = GetWorld()->SpawnActor<ARangedWeapon>();
	Sword = SwordToEquip;
	Gun = GunToEquip;
	if (SwordMesh)
	{
		Sword->Mesh->SetStaticMesh(SwordMesh);
	}
	if (GunMesh)
	{
		Gun->Mesh->SetStaticMesh(GunMesh);
	}
	Sword->Equip(this);
	Gun->Equip(this);
	if (InCombat)
	{
		EnterCombat();
	}
	else
	{
		ExitCombat();
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::Walk);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Leap);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMainCharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::CrouchStart);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMainCharacter::CrouchEnd);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMainCharacter::Slash);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMainCharacter::Shoot);
	PlayerInputComponent->BindAction("Buster", IE_Pressed, this, &AMainCharacter::ExecuteBuster);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AMainCharacter::Dash);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Lookup", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Lookup", this, &AMainCharacter::LookUpAtRate);

}

void AMainCharacter::MoveForward(float Value)
{
	if (!IsAttacking)
	{
		if ((Controller != nullptr) && Value != 0.0f)
		{
			// find the forward direction
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			AddMovementInput(Direction, Value);
		}
	}
}

void AMainCharacter::MoveRight(float Value)
{
	if (!IsAttacking)
	{
		// find the forward direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	if (!IsAttacking)
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AMainCharacter::LookUpAtRate(float Rate)
{
	if (!IsAttacking)
	{
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void AMainCharacter::CrouchStart()
{
	GetCharacterMovement()->Crouch();
	GetCharacterMovement()->bWantsToCrouch = true;
	UE_LOG(LogTemp, Warning, TEXT("Character crouching!"));
}

void AMainCharacter::CrouchEnd()
{
	GetCharacterMovement()->UnCrouch();
	GetCharacterMovement()->bWantsToCrouch = false;
	UE_LOG(LogTemp, Warning, TEXT("Character not crouching!"));
}

void AMainCharacter::Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseRunSpeed;
}

void AMainCharacter::Walk()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AMainCharacter::Dash()
{
	if (CanDash && !IsAttacking)
	{
		CanDash = false;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && DashMontage)
		{
			AnimInstance->Montage_Play(DashMontage, 1.35f);
		}
		GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
		LaunchCharacter(FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0).GetSafeNormal() * DashDistance, true, true);
		GetMesh()->ToggleVisibility();
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &AMainCharacter::StopDash, DashStop, false);
	}

}

void AMainCharacter::StopDash()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetMesh()->ToggleVisibility();
	BurstParticle->ToggleActive();
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AMainCharacter::ResetDash, DashCooldown, false);

}

void AMainCharacter::ResetDash()
{
	CanDash = true;
	GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
}

void AMainCharacter::DelayDash()
{
	if (CanDash)
	{
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &AMainCharacter::Dash, DashDelay, false);
	}
}

void AMainCharacter::EndAirBuster()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.25f);
		GetCharacterMovement()->GravityScale = 2.0f;
		AnimInstance->Montage_Play(EnderMontage, 1.2f);
		AnimInstance->Montage_JumpToSection("AirEnder", EnderMontage);
		CurrentComboStatus = EComboStatus::ECS_Opener;
		AirBoost(500.0f);
		CurrentBusterLevel--;
		if (CurrentBusterLevel <= 0)
		{
			CurrentBusterLevel = 0;
		}
		ResetDash();
	}
}

void AMainCharacter::Leap()
{
	if (!IsAttacking)
	{
		Jump();
		CurrentComboStatus = EComboStatus::ECS_Opener;
	}
}

void AMainCharacter::EnterCombat()
{
	BurstParticle->ToggleActive();
	Sword->Equip(this);
}

void AMainCharacter::ExitCombat()
{
	Sword->Unequip(this);
}

void AMainCharacter::Slash()
{
	if (!IsAttacking)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!GetCharacterMovement()->IsFalling())
			{
				IsAttacking = true;
				switch (CurrentComboStatus)
				{
				case EComboStatus::ECS_Opener:
					AnimInstance->Montage_Play(OpenerMontage, 1.5f);
					AnimInstance->Montage_JumpToSection("GroundSwordOpener", OpenerMontage);
					CurrentComboStatus = EComboStatus::ECS_Linker1;
					break;
				case EComboStatus::ECS_Linker1:
					AnimInstance->Montage_Play(Linker1Montage, 1.5f);
					AnimInstance->Montage_JumpToSection("GroundSwordLinker1", Linker1Montage);
					CurrentComboStatus = EComboStatus::ECS_Ender;
					break;
				case EComboStatus::ECS_Linker2:
					AnimInstance->Montage_Play(Linker2Montage, 1.5f);
					AnimInstance->Montage_JumpToSection("GroundSwordLinker2", Linker2Montage);
					CurrentComboStatus = EComboStatus::ECS_Ender;
					break;
				case EComboStatus::ECS_Ender:
					AnimInstance->Montage_Play(EnderMontage, 1.5f);
					AnimInstance->Montage_JumpToSection("GroundSwordEnder", EnderMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					CurrentBusterLevel++;
					if (CurrentBusterLevel >= 3)
					{
						CurrentBusterLevel = 3;
					}
					break;
				default:
					break;
				}
			}
			else
			{
				float JumpBoost = 500.0f;
				IsAttacking = true;
				switch (CurrentComboStatus)
				{
				case EComboStatus::ECS_Opener:
					AnimInstance->Montage_Play(OpenerMontage, 1.5f);
					AnimInstance->Montage_JumpToSection("AirOpener", OpenerMontage);
					CurrentComboStatus = EComboStatus::ECS_Linker1;
					AirBoost(JumpBoost);
					break;
				case EComboStatus::ECS_Linker1:
					AnimInstance->Montage_Play(Linker1Montage, 1.5f);
					AnimInstance->Montage_JumpToSection("AirLinker1", Linker1Montage);
					CurrentComboStatus = EComboStatus::ECS_Ender;
					AirBoost(JumpBoost);
					break;
				case EComboStatus::ECS_Linker2:
					AnimInstance->Montage_Play(Linker2Montage, 1.5f);
					AnimInstance->Montage_JumpToSection("AirLinker2", Linker2Montage);
					CurrentComboStatus = EComboStatus::ECS_Ender;
					AirBoost(JumpBoost);
					break;
				case EComboStatus::ECS_Ender:
					AnimInstance->Montage_Play(EnderMontage, 1.5f);
					AnimInstance->Montage_JumpToSection("AirEnder", EnderMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					AirBoost(JumpBoost);
					CurrentBusterLevel++;
					if (CurrentBusterLevel >= 3)
					{
						CurrentBusterLevel = 3;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

void AMainCharacter::Shoot()
{
	if (!IsAttacking)
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			GetCharacterMovement()->StopMovementImmediately();
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				IsAttacking = true;
				switch (CurrentComboStatus)
				{
				case EComboStatus::ECS_Opener:
					AnimInstance->Montage_Play(OpenerMontage, 1.5f);
					AnimInstance->Montage_JumpToSection("GroundGunOpener", OpenerMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					break;
				case EComboStatus::ECS_Linker1:
					AnimInstance->Montage_Play(Linker1Montage, 1.5f);
					AnimInstance->Montage_JumpToSection("GroundGunLinker1", Linker1Montage);
					CurrentComboStatus = EComboStatus::ECS_Ender;
					break;
				case EComboStatus::ECS_Linker2:
					AnimInstance->Montage_Play(Linker2Montage, 1.2f);
					AnimInstance->Montage_JumpToSection("GroundGunLinker2", Linker2Montage);
					CurrentComboStatus = EComboStatus::ECS_Ender;
					break;
				case EComboStatus::ECS_Ender:
					AnimInstance->Montage_Play(EnderMontage, 1.0f);
					AnimInstance->Montage_JumpToSection("GroundGunEnder", EnderMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					CurrentBusterLevel++;
					if (CurrentBusterLevel >= 3)
					{
						CurrentBusterLevel = 3;
					}
					break;
				default:
					break;
				}
			}
		}
		else
		{

		}
	}
}

void AMainCharacter::ExecuteBuster()
{
	if (!IsAttacking)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!GetCharacterMovement()->IsFalling())
			{
				IsAttacking = true;
				switch (CurrentBusterLevel)
				{
				case 0:
					AttackEnd();
					break;
				case 1:
					AnimInstance->Montage_Play(BusterMontage, 1.0f);
					AnimInstance->Montage_JumpToSection("Buster_Level1", BusterMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					CurrentBusterLevel = 0;
					break;
				case 2:
					AnimInstance->Montage_Play(BusterMontage, 1.0f);
					AnimInstance->Montage_JumpToSection("Buster_Level2", BusterMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					CurrentBusterLevel = 0;
					break;
				case 3:
					AnimInstance->Montage_Play(BusterMontage, 1.0f);
					AnimInstance->Montage_JumpToSection("Buster_Level3", BusterMontage);
					CurrentComboStatus = EComboStatus::ECS_Opener;
					CurrentBusterLevel = 0;
					break;
				default:
					break;
				}
			}
			else
			{
				if (CurrentComboStatus == EComboStatus::ECS_Ender && CurrentBusterLevel > 0)
				{
					AirBoost(10.0f);
					GetCharacterMovement()->GravityScale = 0.1f;
					IsAttacking = true;
					AnimInstance->Montage_Play(BusterMontage, 1.0f);
					AnimInstance->Montage_JumpToSection("Level3", BusterMontage);
					GetWorldTimerManager().SetTimer(UnusedHandle, this, &AMainCharacter::EndAirBuster, 0.5f, false);
				}
			}
		}
	}
}

void AMainCharacter::AirBoost(float JumpBoostMultiplier)
{
	LaunchCharacter(FVector(0.0f, 0.0f, JumpBoostMultiplier), true, true);
}

void AMainCharacter::AttackEnd()
{
	IsAttacking = false;
}



