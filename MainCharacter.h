// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class EComboStatus : uint8
{
	ECS_Opener UMETA(DisplayName = "Opener"),
	ECS_Linker1 UMETA(DisplayName = "FirstLinker"),
	ECS_Linker2 UMETA(DisplayName = "SecondLinker"),
	ECS_Ender UMETA(DisplayName = "Ender")
};



UCLASS()
class THIRDPERSONTEST_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	//Follow Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* DashMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* OpenerMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* Linker1Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* Linker2Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* EnderMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* BusterMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anims")
	EComboStatus CurrentComboStatus;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anims")
	int CurrentBusterLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	class UNiagaraComponent* BurstParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapons)
	class AMeleeWeapon* Sword;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapons)
	class ARangedWeapon* Gun;


	//Base turn rates to scale turning functions for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float BaseWalkSpeed = 600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float BaseRunSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float DashDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float DashCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	bool CanDash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float DashStop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float DashDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims")
	bool IsDashing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	bool InCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool IsAttacking;

	UPROPERTY()
	FTimerHandle UnusedHandle;
	UPROPERTY()
	USkeletalMeshComponent* MyMesh;
	UPROPERTY()
	UStaticMesh* SwordMesh;
	UPROPERTY()
	UStaticMesh* GunMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called for forwards/backwards input
	void MoveForward(float Value);

	// Called for side to side input
	void MoveRight(float Value);
	
	/** Called via input to turn at a given rate
	 * @param Rate This is a normalized rate, e.g. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate
	 * @param Rate This is a normalized rate, e.g. 1.0 means 100% of desired look up/down rate
	 */
	void LookUpAtRate(float Rate);

	void CrouchStart();
	void CrouchEnd();

	UFUNCTION()
	void Sprint();
	UFUNCTION()
	void Walk();
	UFUNCTION()
	void Dash();
	UFUNCTION()
	void StopDash();
	UFUNCTION()
	void ResetDash();
	UFUNCTION()
	void DelayDash();
	UFUNCTION()
	void EndAirBuster();

	UFUNCTION()
	void Leap();

	UFUNCTION()
	void EnterCombat();
	UFUNCTION()
	void ExitCombat();


	UFUNCTION()
	void Slash();
	UFUNCTION()
	void Shoot();
	UFUNCTION()
	void ExecuteBuster();
	UFUNCTION()
	void AirBoost(float JumpBoostMultiplier);
	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};
