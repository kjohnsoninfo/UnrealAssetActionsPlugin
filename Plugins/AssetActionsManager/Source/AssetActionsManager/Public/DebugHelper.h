#pragma once

#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

namespace DebugHelper 
{
	static void Print(const FString& Message, const FColor& Color = FColor::Green)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.8f, Color, Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type MessageDialogBox(EAppMsgType::Type MsgType, const FString& Message, const FString& MsgTitle = "Warning")
	{
		return FMessageDialog::Open(MsgType, FText::FromString(Message), FText::FromString(MsgTitle));
	}

	static void NotificationPopup(const FString& Message)
	{
		FNotificationInfo NotificationInfo(FText::FromString(Message));
		NotificationInfo.bUseLargeFont = true;
		NotificationInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
}