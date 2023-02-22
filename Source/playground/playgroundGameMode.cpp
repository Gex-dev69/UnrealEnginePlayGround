// Copyright Epic Games, Inc. All Rights Reserved.

#include "playgroundGameMode.h"
#include "playgroundCharacter.h"
#include "UObject/ConstructorHelpers.h"

AplaygroundGameMode::AplaygroundGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
