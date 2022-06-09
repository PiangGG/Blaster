// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h" 

#include "Blaster/Blaster.h"
#include "Blaster/BlasterCompnent/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ABlasterCharacter::ABlasterCharacter()
{ 	
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera ->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//bUseControllerRotationPitch = false;
	//bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore );
	GetMesh()->SetCollisionObjectType(ECC_SkeletaMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.0f,0.0f,850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	
	TimeSinceLastMovementReplication = 0.0f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole()>ENetRole::ROLE_SimulatedProxy&&IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication>0.25)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pich();
	}
	
	HideCameraIfCharacterClose();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward",this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this, &ABlasterCharacter::LookUp);
	
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip",IE_Pressed,this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Pressed,this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Released,this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire",IE_Released,this, &ABlasterCharacter::FireBUttonReleased);

}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABlasterCharacter,OverlappingWeapon,COND_OwnerOnly)
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this; 
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon ==nullptr)return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance&&FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming?FName("RifleAim"):FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName); 
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon ==nullptr)return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance&&HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName); 
	}
}

void ABlasterCharacter::MoveForward(float var)
{
	if (Controller!=nullptr&&var != 0.0f)
	{
		const FRotator YawRotation(0.0f,Controller->GetControlRotation().Yaw,0.0f);
		const FVector Dirction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Dirction,var);
	}
}

void ABlasterCharacter::MoveRight(float var)
{
	if (Controller!=nullptr&&var != 0.0f)
	{
		const FRotator YawRotation(0.0f,Controller->GetControlRotation().Yaw,0.0f);
		const FVector Dirction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Dirction,var);
	}
}

void ABlasterCharacter::Turn(float var)
{
	AddControllerYawInput(var);
}

void ABlasterCharacter::LookUp(float var)
{
	AddControllerPitchInput(var);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon); 
		}
		else
		{
			ServerEquipButtonPressed();
		}
		
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat&&Combat->EquippedWeapon == nullptr)return;
	
	
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	float Speed = CalculateSpeed();
	
	if (Speed == 0.0f && !bIsInAir)//站立 没动
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.0f,GetBaseAimRotation().Yaw,0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;

		TurnInPlace(DeltaTime);
	}
	if (Speed>0.0f||bIsInAir) //跑或者跳跃
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.0f,GetBaseAimRotation().Yaw,0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pich();
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0F;
	return  Velocity.Size();
}

void ABlasterCharacter::CalculateAO_Pich()
{
	AO_Pich = GetBaseAimRotation().Pitch;
	if (AO_Pich>90.0f&& !IsLocallyControlled())
	{
		FVector2D InRange(270.0f,360.0f);
		FVector2D OutRange(-90.0f,0.0f);
		AO_Pich = FMath::GetMappedRangeValueClamped(InRange,OutRange,AO_Pich);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr||Combat->EquippedWeapon == nullptr)return;;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();

	if (Speed>0.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	
	ProxyRotation = GetActorRotation();

	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right; 
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left; 
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning; 
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}

}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPresswed(true);
	}
}

void ABlasterCharacter::FireBUttonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPresswed(false);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon *LastOverlappingWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickuoWidget(true);
	}

	if (LastOverlappingWeapon)
	{
		LastOverlappingWeapon->ShowPickuoWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon); 
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw>90.0F)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw<-90.0F)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw,0.0f,DeltaTime,4.0f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw)<15.0F)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.0f,GetBaseAimRotation().Yaw,0.0f);
		}
	}
}

void ABlasterCharacter::MuticastHit_Implementation()
{
	 PlayHitReactMontage();
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())return;
	if ((FollowCamera->GetComponentLocation()-GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter:: SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickuoWidget(false);
	}
	
	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickuoWidget(true);
		}
	}
	
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat&&Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat&&Combat->bAiming); 
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)return FVector();
	return Combat->HitTarget;
}
