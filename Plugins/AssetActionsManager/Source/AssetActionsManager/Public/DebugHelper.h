#pragma once

namespace DebugHelper 
{
	static void Print(const FString& Message, const FColor& Color = FColor::Green)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.8f, Color, Message);
		}
	}

}