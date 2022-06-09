// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InterractWithCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter,public IInterractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);

	UFUNCTION(NetMulticast,Unreliable)
	void MuticastHit();

	virtual void OnRep_ReplicatedMovement() override;
protected:
	virtual void BeginPlay() override;

	void MoveForward(float var);
	void MoveRight(float var);
	void Turn(float var);
	void LookUp(float var);

	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireBUttonReleased();

	void PlayHitReactMontage();

private:
	UPROPERTY(VisibleAnywhere,Category = Camera)
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere,Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon *OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastOverlappingWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();
	
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pich;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditDefaultsOnly,Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditDefaultsOnly,Category = Combat)
	class UAnimMontage* HitReactMontage;

	
	
	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.0f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float  ProxyYaw;
	float TimeSinceLastMovementReplication;	
	float CalculateSpeed();
	void CalculateAO_Pich();

	/**
	 *Player health 
	 */
	UPROPERTY(EditAnywhere,Category= "Player Stats")
	float MaxHealth = 100.0f;
	UPROPERTY(ReplicatedUsing = OnRep_Health,VisibleAnywhere,Category = "Player Stats")
	float Health = 100.0f;
	UFUNCTION()
	void OnRep_Health();
public:

	void SetOverlappingWeapon(AWeapon* Weapon);
	
	bool IsWeaponEquipped();

	bool IsAiming();

	FORCEINLINE float GetAO_Yaw(){return AO_Yaw;};	
	FORCEINLINE float GetAO_Pich(){return AO_Pich;};

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace()const{return TurningInPlace;}
	FVector GetHitTarget()const;
	FORCEINLINE UCameraComponent* GetFollowCamera()const{return  FollowCamera;}
	FORCEINLINE bool ShouldRotateRootBone()const {return bRotateRootBone;}
	
};


