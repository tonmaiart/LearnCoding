// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SShotReaderWidgetTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SShotReaderWidgetTab){}


	//SLATE_ARGUMENT()

	SLATE_END_ARGS()

private:
	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

public:
	void Construct(const FArguments& InArgs);
};