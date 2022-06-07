// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectingImpulse = 10.0f; 
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this,&ACasing::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector()*ShellEjectingImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ShellSound,GetActorLocation());
	}
	Destroy();
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle_Destroy,this,&ACasing::TiseDestroyed,1.0f,false);
	
}

void ACasing::TiseDestroyed()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Destroy);
	Destroy();
}


