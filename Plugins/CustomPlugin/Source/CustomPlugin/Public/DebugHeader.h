#pragma once

#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace DebugHeader
{

	static void Print(const FString& Message, const FColor& Color = FColor::White)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, Color, Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message)

	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true)
	{
		if (bShowMsgAsWarning == true) {
			return FMessageDialog::Open(MsgType, FText::FromString(Message), FText::FromString(TEXT("Warning")));

		}
		else
		{
			return FMessageDialog::Open(MsgType, FText::FromString(Message));
		}
	}
	 
	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}

}
