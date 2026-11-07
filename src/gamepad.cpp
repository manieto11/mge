#include "mge/input/gamepad.hpp"
#include "utils.h"

#define MAX_GAMEPADS 16

namespace mge
{
    GLFWgamepadstate prevStates[MAX_GAMEPADS];
    GLFWgamepadstate currStates[MAX_GAMEPADS];

    Gamepad::Gamepad(int index) : index(index) {}

    void Gamepad::Initialize() const
    {
        glfwGetGamepadState(index, &prevStates[index]);
        glfwGetGamepadState(index, &currStates[index]);
    }

    void Gamepad::Update() const
    {
        prevStates[index] = currStates[index];
        glfwGetGamepadState(index, &currStates[index]);
    }

    bool Gamepad::IsAvailable() const
    {
        return index >= 0 and glfwJoystickIsGamepad(index);
    }

    bool Gamepad::IsButtonDown(GamepadButton button) const
    {
        if (index < 0)
            return false;

        return currStates[index].buttons[(int)button];
    }

    bool Gamepad::IsButtonPressed(GamepadButton button) const
    {
        if (index < 0)
            return false;

        return currStates[index].buttons[(int)button] && !prevStates[index].buttons[(int)button];
    }

    bool Gamepad::IsButtonReleased(GamepadButton button) const
    {
        if (index < 0)
            return false;

        return !currStates[index].buttons[(int)button] && prevStates[index].buttons[(int)button];
    }

    float Gamepad::GetAxis(GamepadAxis axis) const
    {
        if (index < 0)
            return 0.0f;

        return currStates[index].axes[(int)axis];
    }

    int Gamepad::GetIndex() const
    {
        return index;
    }
} // namespace mge
