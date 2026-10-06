#include "Core/Globals.h"
#include "Core/Application.h"
#include "Modules/ModuleInput.h"

#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Input/GamePad.h"

ModuleInput::ModuleInput(HWND hWnd)
{
    keyboard = std::make_unique<Keyboard>();
    mouse = std::make_unique<Mouse>();
    gamePad = std::make_unique<GamePad>();

    mouse->SetWindow(hWnd);
}
