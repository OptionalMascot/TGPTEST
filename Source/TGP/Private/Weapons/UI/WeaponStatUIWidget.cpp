// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/UI/WeaponStatUIWidget.h"
#include "Components/TextBlock.h"

void UWeaponStatUIWidget::SetText(const FString text)
{
	FText textLabel = FText::FromString(text);

	StatLabel->SetText(textLabel);
}
