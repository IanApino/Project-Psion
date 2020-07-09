// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MainCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"

ARangedWeapon::ARangedWeapon()
{
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	Mesh->SetSimulatePhysics(false);
}

void ARangedWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	// 	if (OtherActor)
	// 	{
	// 		AMainCharacter* Character = Cast<AMainCharacter>(OtherActor);
	// 		if (Character)
	// 		{
	// 			Equip(Character);
	// 		}
	// 	}
}

void ARangedWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void ARangedWeapon::Equip(AMainCharacter* character)
{
	if (character)
	{
		const USkeletalMeshSocket* LeftHandSocket = character->GetMesh()->GetSocketByName("GunSocket");
		if (LeftHandSocket)
		{
			LeftHandSocket->AttachActor(this, character->GetMesh());
			bRotate = false;
		}
	}
	Mesh->SetVisibility(true);
}

void ARangedWeapon::Unequip(AMainCharacter* character)
{
	const USkeletalMeshSocket* BackSocket = character->GetMesh()->GetSocketByName("SheathGunSocket");
	if (BackSocket)
	{
		BackSocket->AttachActor(this, character->GetMesh());
		bRotate = false;
	}
	Mesh->SetVisibility(false);
}