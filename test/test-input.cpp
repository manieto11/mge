#include "GLFW/glfw3.h"
#include "mge/input.hpp"

#include <iostream>

int main()
{
    glfwInit();

    GLFWwindow *window = glfwCreateWindow(800, 600, "test input", nullptr, nullptr);

    mge::Input::Initialize(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        mge::Input::Update();

        if (mge::Input::KeyPressed(mge::KeyboardKey::ESCAPE))
            glfwSetWindowShouldClose(window, true);
        if (mge::Input::KeyPressed(mge::KeyboardKey::SPACE))
            std::cout << "Key space pressed.\n";
        if (mge::Input::MousePressed(mge::MouseButton::LEFT))
            std::cout << "Mouse button left pressed.\n";
        if (mge::Input::GetGamepad(0).IsButtonPressed(mge::GamepadButton::LSB))
            std::cout << "Gamepad button LSB pressed.\n";

        // std::cout << "Gamepad RT axis = " << mge::Input::GetGamepad(0).GetAxis(mge::GamepadAxis::RT) << ".\n";
    }

    mge::Input::Finalize();

    glfwTerminate();
}