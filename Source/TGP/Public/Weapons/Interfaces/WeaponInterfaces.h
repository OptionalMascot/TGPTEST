// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponInterfaces.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDealDamage : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TGP_API IDealDamage
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};

UINTERFACE(MinimalAPI)
class UHealth : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API IHealth
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
};

UINTERFACE(MinimalAPI)
class UCanHitScan : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API ICanHitScan
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
};

UINTERFACE(MinimalAPI)
class UHasAmmo : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API IHasAmmo
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
};

UINTERFACE(MinimalAPI)
class UWaitTimer : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TGP_API IWaitTimer
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
};
