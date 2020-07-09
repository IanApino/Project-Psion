// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MainCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"


AMeleeWeapon::AMeleeWeapon()
{
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	Mesh->SetSimulatePhysics(false);
}


void AMeleeWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void AMeleeWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void AMeleeWeapon::Equip(AMainCharacter* character)
{
	if (character)
	{
		const USkeletalMeshSocket* RightHandSocket = character->GetMesh()->GetSocketByName("SwordSocket");
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, character->GetMesh());
			bRotate = false;
		}
	}
	Mesh->SetVisibility(true);
}

void AMeleeWeapon::Unequip(AMainCharacter* character)
{
	const USkeletalMeshSocket* BackSocket = character->GetMesh()->GetSocketByName("SheathSwordSocket");
	if (BackSocket)
	{
		BackSocket->AttachActor(this, character->GetMesh());
		bRotate = false;
	}
	Mesh->SetVisibility(false);
}

