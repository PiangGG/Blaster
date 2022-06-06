// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisonBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisonBox);
	CollisonBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisonBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisonBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
	CollisonBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,ECollisionResponse::ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

