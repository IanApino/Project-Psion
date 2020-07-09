// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONTEST_API AMeleeWeapon : public AItem
{
	GENERATED_BODY()
public:

    AMeleeWeapon();

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION()
	void Equip(class AMainCharacter* character);
	UFUNCTION()
	void Unequip(class AMainCharacter* character);
};
