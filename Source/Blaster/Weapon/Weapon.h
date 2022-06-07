 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial		UMETA(DisplayName = "Initial State"),
	EWS_Equipped	UMETA(DisplayName = "Equipped"),
	EWS_Dropped		UMETA(DisplayName = "Dropped"),
	
	EWS_Max			UMETA(DisplayName = "DefaultMax"),
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	void ShowPickuoWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);

	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsBottom;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor*OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,AActor*OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);
private:
	UPROPERTY(VisibleAnywhere,Category = "Wepaon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere,Category = "Wepaon Properties")
	class  USphereComponent* AreaSphere;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere,Category = "Wepaon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();
	
	UPROPERTY(VisibleAnywhere,Category = "Wepaon Properties")
	class UWidgetComponent* PickupWidget;
	
	UPROPERTY(EditAnywhere,Category = "Wepaon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere,Category = "Wepaon")
	TSubclassOf<class ACasing> CasingClass;
public:	

	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere()const {return AreaSphere;};
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh(){return WeaponMesh;}
};
